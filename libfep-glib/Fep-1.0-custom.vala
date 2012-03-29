namespace Fep {
	[CCode (cheader_filename = "libfep-glib/libfep-glib.h", has_destroy_function = false)]
	public struct GAttribute {
		public Fep.GAttrType type;
		public uint value;
		public uint start_index;
		public uint end_index;
	}
}
