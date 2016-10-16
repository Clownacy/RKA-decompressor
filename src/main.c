// LZSS decompressor for Rocket Knight Adventures, made by Clownacy
// License: I dunno, this was based on reverse-engineered code anyway

#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 0

void doRawData(void);
void doDictionary(void);
void writeByte(uint8_t value);
uint8_t readByte(void);
uint16_t readWord(void);

uint8_t dictionary[0x400];
int dictionary_index;

int size_of_uncompressed_file;

FILE *src_file, *dst_file;

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printf("LZSS decompressor for Rocket Knight Adventures, made by Clownacy\n\nUsage: %s path_to_source path_to_destination\n", basename(argv[0]));
		exit(EXIT_FAILURE);
	}

	src_file = fopen(argv[1], "rb");
	if (src_file == NULL)
	{
		printf("Error: File '%s' could not be opened.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	dst_file = fopen((argc >= 3 ? argv[2] : "decompressed.bin"), "wb");
	if (dst_file == NULL)
	{
		printf("Error: Could not open destination file.\n");
		exit(EXIT_FAILURE);
	}

	// Initialise dictionary
	for (int i=0; i < 0x3C0; ++i)
		dictionary[i] = 0x20;

	dictionary_index = 0x3C0;

	fseek(src_file, 0, SEEK_END);
	const int src_filesize = ftell(src_file);
	rewind(src_file);

	size_of_uncompressed_file = readWord();
	const int end_of_file = readWord() + ftell(src_file);

	if (src_filesize > end_of_file)
		printf("Warning: Compressed file is larger than expected (is 0x%X; expected 0x%X).\n", src_filesize,  end_of_file);
	else if (src_filesize < end_of_file)
		printf("Warning: Compressed file is smaller than expected (is 0x%X; expected 0x%X).\n", src_filesize,  end_of_file);

	#if DEBUG
	printf("Size of uncompressed data = 0x%X bytes\nSize of compressed data = 0x%X bytes\n", size_of_uncompressed_file, end_of_file);
	#endif

	while (ftell(src_file) < end_of_file)
	{
		#if DEBUG
		printf("\nGetting new description field...\n");
		#endif
		uint8_t description_field = readByte();

		for (int i=0; (i < 8*sizeof(description_field)) && (ftell(src_file) < end_of_file); ++i)
		{
			if ((description_field&(1<<i)) != 0)
				doRawData();
			else
				doDictionary();
		}
	}

	if (ftell(dst_file) > size_of_uncompressed_file)
		printf("Warning: Decompressed file is larger than expected (is 0x%X; expected 0x%X).\n", ftell(dst_file),  size_of_uncompressed_file);
	else if (ftell(dst_file) < size_of_uncompressed_file)
		printf("Warning: Decompressed file is smaller than expected (is 0x%X; expected 0x%X).\n", ftell(dst_file),  size_of_uncompressed_file);

	fclose(src_file);
	fclose(dst_file);

	printf("Decompressed to '%s' successfully.\n", (argc >= 3 ? argv[2] : "decompressed.bin"));

	return EXIT_SUCCESS;
}

void doRawData(void)
{
	#if DEBUG
	printf("\nDoing raw data (1 byte)...\n");
	#endif
	writeByte(readByte());
}

void doDictionary(void)
{
	uint16_t dictionary_entry = readWord();
	const int length = (dictionary_entry&0xFC00)>>0xA;
	int index = dictionary_entry&0x3FF;
	#if DEBUG
	printf("\nDoing dictionary data...\n");
	printf("  Length = 0x%X (0x%X bytes)\n", length, length+1);
	printf("  Index = 0x%X\n", index);
	#endif

	for (int i=0; i < length+1; ++i)
	{
		writeByte(dictionary[index++]);
		index &= 0x3FF;
	}
}

void writeByte(uint8_t value)
{
	fputc(value, dst_file);
	dictionary[dictionary_index++] = value;
	dictionary_index &= 0x3FF;
}

uint8_t readByte(void)
{
	int value = fgetc(src_file);
	if (value == EOF)
	{
		printf("Error: Reached end-of-file prematurely. Input file appears to be incomplete.\n");
		exit(EXIT_FAILURE);
	}

	return value;
}

uint16_t readWord(void)
{
	return (readByte()<<8)|readByte();
}
