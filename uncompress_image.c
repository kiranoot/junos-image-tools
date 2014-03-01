/* File format notes:
uint32 Number of compressed chunks, must be less than 65535
uint32 Uncompressed size
<compressed chunk header>*num chunks
<compressed chunks>*num chunks

chunk header:
uint32 file offset
uint32 chunk size
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#ifndef _WIN32
#include <endian.h>
#endif
#include "inflate.h"

FILE* input_file = 0;
FILE* output_file = 0;

static uint32_t swap(uint32_t in)
{
#if _WIN32
	unsigned char* pieces = (unsigned char*)&in;
	return (uint32_t)((uint32_t)pieces[3]|(uint32_t)pieces[2]>>8|(uint32_t)pieces[1]>>16|(uint32_t)pieces[0]>>24);
#else
	return be32toh(in);
#endif
}

typedef struct chunk_header
{
	uint32_t file_offset;
	uint32_t chunk_size;
} chunk_header;

static void fatal(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	exit(1);
}

static int get_byte(void* ignored)
{
	uint8_t byte = 0;
	fread(&byte, 1, 1, input_file);
	return byte;
}

static int put_bytes(void* ignored, unsigned char* buffer, unsigned long size)
{
	fwrite(buffer, 1, size, output_file);
	return 0;
}

static int inflate_zchunk(uint32_t offset, uint32_t size)
{
	fseek(input_file, offset, SEEK_SET);
	if(size == 65536)
	{
		char buffer[65536];
		fread(&buffer, 1, 65536, input_file);
		fwrite(&buffer, 1, 65536, output_file);
		return 0;
	}

	struct inflate inf;
	memset(&inf, 0, sizeof(inf));
	inf.gz_input = get_byte;
	inf.gz_output = put_bytes;
	inf.gz_slide = (unsigned char*)malloc(65536);
	xinflate(&inf);
	free(inf.gz_slide);
	return 0;
}

int main(int argc, char* argv[])
{
	if(argc < 3)
		fatal("Usage: %s <input image> <output image>\n");

	input_file = fopen(argv[1], "rb");
	output_file = fopen(argv[2], "wb");
	if(!input_file)
		fatal("Could not open input file %s\n", input_file);
	if(!output_file)
		fatal("Could not open output file %s\n", output_file);

	uint32_t num_chunks = 0;
	fread(&num_chunks, 1, 4, input_file);
	num_chunks = swap(num_chunks);
	fseek(input_file, 4, SEEK_CUR);
	printf("Number of chunks: %u\n", num_chunks);

	chunk_header* chunk_headers = malloc(sizeof(chunk_header)*num_chunks);
	int chunk_id = 0;
	for(chunk_id = 0; chunk_id < num_chunks; ++chunk_id)
	{
		uint32_t raw_data[2];
		fread(&raw_data, 1, 8, input_file);
		chunk_headers[chunk_id].file_offset = swap(raw_data[0]);
		chunk_headers[chunk_id].chunk_size = swap(raw_data[1]);
		//printf("Chunk %d: offset: %u size: %u\n", chunk_id, chunk_headers[chunk_id].file_offset, chunk_headers[chunk_id].chunk_size);
	}

	for(chunk_id = 0; chunk_id < num_chunks; ++chunk_id)
	{
		//printf("Uncompressing chunk %d\n", chunk_id);
		printf("Chunk %d: offset: %u size: %u\n", chunk_id, chunk_headers[chunk_id].file_offset, chunk_headers[chunk_id].chunk_size);
		inflate_zchunk(chunk_headers[chunk_id].file_offset, chunk_headers[chunk_id].chunk_size);
	}

	return 0;
}