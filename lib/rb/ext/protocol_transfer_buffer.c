
#include <ruby.h>
#include "protocol_transfer.h"

#define BUFFER_LEN 4096
#define DEBUG 0

#if DEBUG
  #define DEBUG_FUNCTION_ENTRY() printf("buffered %s\n", __FUNCTION__);
  #define DEBUG_FUNCTION_PROGRESS() printf("%s, %s:%d\n", __FILE__, __FUNCTION__, __LINE__);
#else
  #define DEBUG_FUNCTION_ENTRY()
  #define DEBUG_FUNCTION_PROGRESS() 
#endif


//#define LOG_FUNC()

struct _buffer_data;
typedef struct _buffer_data buffer_data;

static	ID read_into_buffer_method_id;
static	ID read_all_method_id;
static 	ID read_byte_method_id;
static 	ID write_method_id;
static 	ID available_method_id;



struct _buffer_data
{
	char* write_buffer;
	char* read_buffer;

	int write_bufer_idx;
	int read_buffer_idx;
	int read_buffer_sz;

	VALUE transport;
	VALUE strbuf;
};

static int protocol_read(protocol_transfer* pt, char* buffer, int length);
static void protocol_write(protocol_transfer* pt, char* buf, int length);
static void protocol_flush(protocol_transfer* pt);




static void protocol_free(protocol_transfer* pt)
{
	if (pt != NULL) 
	{
		buffer_data* data = (buffer_data*)pt->data;	      		

		if (data != NULL)
		{
			free(data->write_buffer);
			free(data->read_buffer);
		}

		free(data);
	}
	free(pt);
}

#define MIN(a,b) ((a)<(b)?(a):(b))

static int protocol_read(protocol_transfer* pt, char* buffer, int length)
{
	DEBUG_FUNCTION_ENTRY();
	buffer_data* data = (buffer_data*)pt->data;

	if (length == 0) return 0;

	//Do we have data in the read buffer?
	if (data->read_buffer_idx == data->read_buffer_sz)
	{
		VALUE buf;
		//No, let's fetch some
		int available = NUM2INT(rb_funcall(data->transport, available_method_id, 0));
		int read_sz = (available < BUFFER_LEN) ? available : BUFFER_LEN;

		if (read_sz > RSTRING_LEN(data->strbuf))
		{
			buf = rb_funcall(data->transport, read_all_method_id, 1, INT2FIX(read_sz));
		}
		else
		{
			rb_funcall(data->transport, read_into_buffer_method_id, 2, data->strbuf, INT2FIX(read_sz));
			buf = data->strbuf;
		}

		data->read_buffer_idx = 0;
		data->read_buffer_sz = read_sz;
		memcpy(data->read_buffer, RSTRING_PTR(buf), read_sz);
	}

	int rsz = MIN(length, data->read_buffer_sz - data->read_buffer_idx);
	memcpy(buffer, data->read_buffer + data->read_buffer_idx, rsz);
	data->read_buffer_idx+= rsz;

	//Make sure we read everything that is requested, even if the buffer holds *some* data, but not enough.
	return rsz + protocol_read(pt, buffer + rsz, length - rsz);
}


static void protocol_write_byte(protocol_transfer* pt, int b)
{
	buffer_data* data = (buffer_data*)pt->data;	      
	data->write_buffer[data->write_bufer_idx++] = b;

	if (data->write_bufer_idx >= BUFFER_LEN) protocol_flush(pt);
}



static void protocol_write(protocol_transfer* pt, char* buf, int length)
{
	int i;

	for(i=0;i<length;i++)
		protocol_write_byte(pt, buf[i]);
}

static void protocol_flush(protocol_transfer* pt)
{
	buffer_data* data = (buffer_data*)pt->data;	      

	rb_funcall(data->transport, write_method_id, 1, rb_str_new(data->write_buffer, data->write_bufer_idx));
	data->write_bufer_idx = 0;
}


void buffer_transfer_initialize(protocol_transfer* pt, VALUE transport, VALUE strbuf)
{
	buffer_data* data = (buffer_data*)malloc(sizeof(buffer_data));


	pt->data = data;
	data->transport = transport;
	read_all_method_id = rb_intern("read_all");;
	write_method_id = rb_intern("write");
	read_byte_method_id = rb_intern("read_byte");
	available_method_id = rb_intern("available");
	read_into_buffer_method_id = rb_intern("read_into_buffer");

	data->write_buffer = malloc(BUFFER_LEN);
	data->read_buffer = malloc(BUFFER_LEN);

	data->write_bufer_idx = 0;
	data->read_buffer_idx = 0;
	data->read_buffer_sz = 0;

	data->strbuf = strbuf;

	pt->free = protocol_free;
	pt->read = protocol_read;
	pt->write = protocol_write;
	pt->flush = protocol_flush;

	pt->initialized = 1;
}

protocol_transfer* buffer_transfer_create(VALUE transport, VALUE strbuf)
{
	protocol_transfer* pt = malloc(sizeof(protocol_transfer));
	buffer_transfer_initialize(pt, transport, strbuf);
	return pt;
}
