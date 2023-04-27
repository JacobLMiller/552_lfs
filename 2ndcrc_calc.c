class LFS:
    def __init__(self, block_size, segment_size):
        self.block_size = block_size
        self.segment_size = segment_size
        self.segment_header_size = 20  # 20 bytes for the Segment Header
        self.metadata_entry_size = 8  # 8 bytes per metadata entry
        self.cleaner_metadata_size = self.segment_size % self.block_size
        self.blocks_per_segment = self.segment_size // self.block_size
        self.metadata_entries_per_segment = self.blocks_per_segment
        self.segments_per_device = 2 ** 64 // self.segment_size
        self.block_address_zero = bytes([0] * self.block_size)

    def block_address_to_segment_number(self, block_address):
        return block_address // self.blocks_per_segment

    def block_address_to_block_offset_in_segment(self, block_address):
        return block_address % self.blocks_per_segment

    def block_address_to_offset_in_device(self, block_address):
        return block_address * self.block_size

    def offset_in_device_to_block_address(self, offset):
        return offset // self.block_size

    def read_block(self, block_address):
        if block_address == 0:
            return self.block_address_zero
        segment_number = self.block_address_to_segment_number(block_address)
        offset_in_segment = self.block_address_to_block_offset_in_segment(block_address)
        offset_in_device = self.block_address_to_offset_in_device(block_address)
        with open("/dev/lfs", "rb") as f:
            f.seek(offset_in_device)
            return f.read(self.block_size)

    def write_block(self, block_address, data):
        segment_number = self.block_address_to_segment_number(block_address)
        offset_in_segment = self.block_address_to_block_offset_in_segment(block_address)
        offset_in_device = self.block_address_to_offset_in_device(block_address)
        with open("/dev/lfs", "rb+") as f:
            f.seek(offset_in_device)
            f.write(data)

    def read_segment_header(self, segment_number):
        block_address = segment_number * self.blocks_per_segment
        return self.read_block(block_address)

    def write_segment_header(self, segment_number, data):
        block_address = segment_number * self.blocks_per_segment
        self.write_block(block_address, data)

    def read_segment_metadata(self, segment_number):
        block_address = segment_number * self.blocks_per_segment
        block_address += 1  # The first block after the Segment Header
        metadata_size = self.metadata_entries_per_segment * self.metadata_entry_size
        metadata = b''
        for i in range(self.metadata_entries_per_segment):
            metadata += self.read_block(block_address)
            block_address += 1
        return metadata

    def write_segment_metadata(self, segment_number, metadata):
        block_address = segment_number * self.blocks_per_segment
        block_address += 1  # The first block after the Segment Header
        for i in range(self.metadata_entries_per_segment):
            self.write_block(block_address, metadata[i*self.metadata_entry_size:(i+1)*self.metadata_entry_size])
            block_address += 1

    def read_cleaner_metadata(self, segment_number):
        block_address = segment_number * self.blocks_per_segment
        block_address += self.blocks_per_segment - 1  # The last block in the segment
        offset_in_block = self.block_size - self.cleaner_metadata_size
        with open("/dev/lfs", "rb") as f:
            f.seek(block_address * self.block_size + offset_in_block)
            return f.read(self.cleaner_metadata_size)

    def write_cleaner_metadata(self, segment_number, data):
        block_address = segment_number
