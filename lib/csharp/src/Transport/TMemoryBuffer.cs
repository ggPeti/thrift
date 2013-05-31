﻿/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

using System.IO;
using System.Reflection;
using Thrift.Protocol;

namespace Thrift.Transport {
	public class TMemoryBuffer : TTransport {
		private readonly MemoryStream byteStream;

		public TMemoryBuffer() {
			byteStream = new MemoryStream();
		}

		public TMemoryBuffer(byte[] buf) {
			byteStream = new MemoryStream(buf);
		}

		public override void Open() {
			/** do nothing **/
		}

		public override void Close() {
			/** do nothing **/
		}

		public override int Read(byte[] buf, int off, int len) {
			return byteStream.Read(buf, off, len);
		}

		public override void Write(byte[] buf, int off, int len) {
			byteStream.Write(buf, off, len);
		}

		public byte[] GetBuffer() {
			return byteStream.ToArray();
		}


		public override bool IsOpen {
			get { return true; }
		}

		public static byte[] Serialize(TBase s) {
			return Serialize(s, new TBinaryProtocol.Factory());
		}

		public static byte[] Serialize(TBase s, TProtocolFactory factory) {
			var t = new TMemoryBuffer();
			var p = factory.GetProtocol(t);

			s.Write(p);

			return t.GetBuffer();
		}

		public static T DeSerialize<T>(byte[] buf) where T : TBase, new() {
			return DeSerialize<T>(buf, new TBinaryProtocol.Factory());
		}

		public static T DeSerialize<T>(byte[] buf, TProtocolFactory factory) where T : TBase, new() {
		       var t = new T();
		       var trans = new TMemoryBuffer(buf);
		       var p = factory.GetProtocol(trans);
		       t.Read(p);
		       return t;
		}

		private bool _IsDisposed;

		// IDisposable
		protected override void Dispose(bool disposing) {
			if (!_IsDisposed) {
				if (disposing) {
					if (byteStream != null)
						byteStream.Dispose();
				}
			}
			_IsDisposed = true;
		}
	}
}