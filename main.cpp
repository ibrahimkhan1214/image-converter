/*
Author: Ibrahim khan
Date: 05/22/2020
Description: Program to convert a BMP file image into its negative.
Input: BMP format image file.
Output: BMP format image file.
IDE: Visual studio 2022.
Compiler: MSVC (Microsoft Visual C++ compiler).

UPDATE NOTES:
->  The previous version of this program was using scanf and fopen
	these functions are now deprecated and scanf_s(scanf_safe), fopen_s(fopen_safe) are now
	recommended with C++11.

->  Fixed the following warnings in the previous version of the program:
	1:Added validations to make sure values in the program could not be zero.
	2:Added null terminator to avoid buffer overflow when reading in the input file name.

->  Fixed the arithmetic overflow error in create image function.
	ERROR:4 byte value was multiplied and assigned to an 8 byte value.
	FIX:casted the value using size_t to a wider type before multiplying.
*/

//preprocessor directives
#include<iostream>		//input, output stream
#include<cstdlib>		//including C/C++ functions
#include<fstream>		//for reading and writting files


using namespace std;
/*
Structure name: BITMAP_HEADER
Purpose: This struct will define the structure for header part of the BMP input file.
Data in bytes is:
name[2]  -> 2 bytes (to store the signature of the file type. ie. BM)
size     -> 4 bytes (size of the header in bytes)
reserved -> 4 bytes (these 4 bytes are reserved,actual value depends on the application that creates the image)
offset   -> 4 bytes (these 4 bytes are starting address of the pixel array)
*/
struct BITMAP_HEADER {
	char name[2];
	unsigned int size;
	int reserved;
	unsigned int offset;
};

/*
Structure name: DIB_HEADER
Purpose: This struct will hold the structure of BITMAP info header data.
Data read is:
size of the header  -> 4 bytes
bitmap width in pixels   -> 4 bytes
bitmap height in pixels  -> 4 bytes
number of color planes	 -> 2 bytes
number of bits per pixel -> 2 bytes
compression method       -> 4 bytes
image size				 -> 4 bytes
extra					 -> 4 bytes (these 4 bytes act as padding to make sure pixel array data is read properly)
*/
struct DIB_HEADER {
	unsigned int header_size;
	signed int width;
	signed int height;
	unsigned short int color_planes;
	unsigned short int bits_per_pixel;
	unsigned int compression;
	unsigned int image_size;
	int extra_bytes[4];
};

/*
Structure name: RGB
Purpose: This struct will define the structure of RGB values of pixels.
Data read is:
red value of pixel -> 1 byte
blue value of pixel -> 1 byte
green value of pixel -> 1 byte
*/
struct RGB {
	signed char blue;
	signed char green;
	signed char red;
};

/*
Structure name: image
Purpose: This struct will define the height, width, RGB structure for the image.
*/
struct image {
	int width;
	int height;
	struct RGB** rgb;	//pointer to the pointer of RGB values since RGB value is 2D
};

/*
Structure name: read_image
Purpose: This structure will read the image in the given file.
*/
struct image read_image(FILE* read_file, int height, int width) {
	struct image file_image;	//image struct to hold the file
	file_image.height = height;
	file_image.width = width;
	int bytes_read;
	int number_rgb;
	if (read_file != NULL && height != 0 && width != 0) {

		file_image.rgb = (struct RGB**)malloc(height * sizeof(void*));		//allocating memory for rgb of size (height x 8 bytes)

		bytes_read = ((24 * width + 31) / 32) * 4;		//calculating row size of pixel array

		number_rgb = (bytes_read / sizeof(struct RGB)) + 1;
		/*for loop to allocate memory to image.rgb and reading the calculated bytes in from the input file.*/
		for (int i = height - 1; i >= 0; i--) {
			file_image.rgb[i] = (struct RGB*)malloc(number_rgb * sizeof(struct RGB));
			if (file_image.rgb[i] != NULL) {
				fread(file_image.rgb[i], 1, bytes_read, read_file);
			}
		}

	}
	return file_image;
};

/*This function will only free the allocated memory at the end of the program.
void free_image(struct image photo) {

	for (int i = photo.height - 1; i >= 0; i--) {
		free(photo.rgb[i]);
		free(photo.rgb);
	}
}
*/
/*This function defines how the RGB values will be altered.
To create a negative image we use the following calculation:
Red = 255 - Red.
Green = 255 - Green.
Blue = 255 - Blue.
returning the RGB value for the pixel.
*/
unsigned char negative(struct RGB rgb) {
	return ((255 - rgb.red) + (255 - rgb.green) + (255 - rgb.blue));

}

/*This function will read in the image and assign the negative pixel RGB values to every pixel value in pixel array
to create the negative image*/
void image_to_negative(struct image photo) {
	for (int i = 0; i < photo.height; i++) {
		for (int j = 0; j < photo.width; j++) {
			photo.rgb[i][j].red = photo.rgb[i][j].green = photo.rgb[i][j].blue = negative(photo.rgb[i][j]);
		}
	}
}
/*This function will open the output file in the project folder and write header, DIB header, and image data to the file*/
int create_negative_image(struct BITMAP_HEADER header, struct DIB_HEADER dibheader, struct image photo) {
	//	FILE* file_out;
	//	fopen_s(&file_out, "temp_file.bmp", "w");	//	opening/creating file
	FILE* file_out = NULL;
	errno_t error;
	error = fopen_s(&file_out, "temp_file.bmp", "w");
	if (error && file_out == NULL) {
		perror("ERROR: Could not create output file.");
		exit(-1);
	}

	image_to_negative(photo);

	fwrite(header.name, 2, 1, file_out);		//write first 2 bytes of signature ('BM').
	fwrite(&header.size, 3 * sizeof(int), 1, file_out);	//write header to the output file.
	fwrite(&dibheader, sizeof(struct DIB_HEADER), 1, file_out);	//write dib header to the output file.

	/*write image pixel array data to the output file*/

	for (int i = photo.height - 1; i >= 0; i--) {
		fwrite(photo.rgb[i], static_cast<size_t>(((24 * photo.width + 31) / 32)) * 4, 1, file_out);
	}

	fclose(file_out);	//close the output file
	return 0;
}

/*This function will check if the header size is 40
It should always be 40 unless the BMP file has extra data in the header file
which will cause problems in reading the BMP file.
*/
void check_header_size(unsigned int size) {
	if (size != 40) {
		perror("ERROR: Header size is not 40, need a clean bmp file to work!");
		exit(-1);
	}
}

/*This validation function will check for bits per pixel
This program is designed to stay consistent with 24 bits per pixel resolution.
*/
void check_bits_per_pixel(unsigned short int bits) {
	if (bits != 24) {
		perror("ERROR: Bits per pixel must be 24 for consistency!");
		exit(-1);
	}
}

/*Validation method to check if the file has been compressed.(The file should not be compressed)
in other words the compression value should always be 0.*/
void check_compression(unsigned int compression) {
	if (compression != 0) {
		perror("ERROR: Image file must not be compressed!");
		exit(-1);
	}
}

/*Check for the signature of the file. In this program we are working with bmp files with signature 'BM'
This signature is the first two alphabets in the BMP file.
*/
void check_BMP_file(char name1, char name2) {
	if (name1 != 'B' || name2 != 'M') {
		perror("ERROR: Could not process file!\n->File format must be for BMP format (.bmp)");
		exit(-1);
	}
}

/*Validation to check color planes of the image. This value must be 1.*/
void check_color_planes(unsigned short int planes) {
	if (planes != 1) {
		perror("ERROR: Color planes must be 1.\nColor planes read from file:" + planes);
		exit(-1);
	}
}

/*Purpose: This function will open the given file.
read the data in the BMP file and display it in the console for developer's understanding.
This function will implement the validations and write the image data read from input file(original_file)
in the output file(temp_file).*/
void open_BM_file() {

	char filename[30] = { '\0' };
	printf("Enter the name of the BMP image file (add .bmp extension): ");
	scanf_s("%29s", filename, (unsigned)_countof(filename));

	FILE* original_file = NULL;
	errno_t error;
	error = fopen_s(&original_file, filename, "rb");

	/*input file validation. Check if it exists*/
	if (error && original_file == NULL) {
		perror("ERROR: Could not open file.\nMake sure file name and location is correct ie. in the project folder");
		exit(-1);
	}

	/*intialize header and DIB header*/
	struct BITMAP_HEADER header;
	struct DIB_HEADER dib_header;

	printf("Header Data**\n------------------\nSize of Header: %zu\n", sizeof(header));

	fread(header.name, 2, 1, original_file);	//read the 'BM' BMP file signature
	fread(&header.size, 3 * sizeof(int), 1, original_file);	//size of the header

	check_BMP_file(header.name[0], header.name[1]);	//validation for BM signature

	printf("first TWO characters: %c%c\n", header.name[0], header.name[1]);
	printf("Size: %d%s\n", header.size, " Bytes");
	printf("offset: %d\n", header.offset);

	fread(&dib_header, sizeof(dib_header), 1, original_file);	//reading in the DIB header from input file

	printf("\n\nDIB HEADER DATA**\n------------------\nSize : %d\n", dib_header.header_size);
	printf("Width: %u\n", dib_header.width);
	printf("height: %u\n", dib_header.height);
	printf("Color Planes(must be 1): %d\n", dib_header.color_planes);
	printf("Bits per pixel: %d\n", dib_header.bits_per_pixel);
	printf("Compression(Should be 0): %u\n", dib_header.compression);
	printf("image_size: %u\n", dib_header.image_size);

	check_bits_per_pixel(dib_header.bits_per_pixel);	//validation for bits per pixel
	check_compression(dib_header.compression);		//validation for compression

	check_header_size(dib_header.header_size);	//validation for header size. It must be 40.
	check_color_planes(dib_header.color_planes);	//validation for color planes

	fseek(original_file, header.offset, SEEK_SET);	//set the seek pointer at the image pixel array data start location.
	struct image img = read_image(original_file, dib_header.height, dib_header.width);	//reading in the image from input file.
	create_negative_image(header, dib_header, img);	//putting the altered negative image in the new BMP output file.s


	fclose(original_file);		//close input file.
	free(img.rgb);		//free image memory allocation
}

/*Main function of the program
This function will only call the open_BM_file() function
review the open_BM_file() documentation for its processing behavior
On successful termination this function will return 0 to the console*/
int main() {

	open_BM_file();

	return 0;
}