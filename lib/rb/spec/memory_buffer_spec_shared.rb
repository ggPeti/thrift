require 'spec_helper'

shared_examples_for 'a memory buffer' do
  before(:each) do
    @trans = transport_class.new
  end

  it 'should write one byte correctly' do
  	@trans.write("a");

  	expect(@trans.available).to eq 1
  end

  it 'should write multiple byte correctly' do
  	@trans.write("abcdefghijklmnopqrst");

  	expect(@trans.available).to eq 20
  end

  it 'should write a combination of bytes correctly' do
  	@trans.write("a");
  	@trans.write("bc");
  	@trans.write("def");
  	@trans.write("g");
  	@trans.write("hijk");
  	@trans.write("l");
  	@trans.write("m");
  	@trans.write("n");
  	@trans.write("opqr");
  	@trans.write("st");

  	expect(@trans.available).to eq 20
 end


 it 'should raise an exception if there are no bytes to read' do
 	expect{@trans.read(1)}.to raise_error(EOFError)
 	expect{@trans.read_byte()}.to raise_error(EOFError)
 	expect{@trans.read_into_buffer(nil, 1)}.to raise_error(EOFError)
 end

 it 'should have the read_byte funciton working correctly' do

 	str = "The red fox jumps over the lazy brown dog";

 	@trans.write(str);

 	str.each_char do |c|
 		expect(@trans.read_byte).to eq c.ord
 	end

 end

 it 'should return the written bytes correctly' do
 	tests = [ "a", "abcd", "abcdefghijklm", "aiuhdeury7843yiru3r4"]

 	tests.each do |t|
 		@trans.write(t);

 		read = @trans.read_all(@trans.available);

 		expect(read).to eq t;
 	end
 end

 it 'should return written bytes read in chunks correctly' do
 	tests = [ "a", "abcd", "abcdefghijklm", "aiuhdeury7843yiru3r4", "reiuhgirtuehruw9ehfi23hwfipuheirhriuoehfoudbe3giupr3hwesipuhipruenrpiuhfipeuwhfpiurwhgipuewhfiuerhrfuohoihqwriotuh3ew8ufherioufhrouhouoiuhofre'"];

 	tests.each do |t|
 		@trans.write(t);



 		i = 0;
 		while i<t.length do
 			expect( t.length - i ).to eq @trans.available

 			r = [16, @trans.available].min;
 			read = @trans.read(r);

 			expect(read).to eq t[i,r];

 			i = i + r;
 		end

 		expect(@trans.available).to eq 0

 	end
 end
end