#include <stdio.h>
#include <math.h>
#include <string.h>

#include <clipt.h>

enum pnm_data_type_e {
	PNM_ASCII = 0,
	PNM_BINARY,
};
typedef enum pnm_data_type_e pnm_data_type_t;

enum pnm_palette_e {
	PNM_BITMAP,
	PNM_GRAYSCALE,
	PNM_RGB24
};
typedef enum pnm_palette_e pnm_palette_t;

sys_result_t pnm_plugin_load();
sys_result_t pnm_plugin_unload();

sys_result_t load_pnm(const char *path, image_data_t **image);

sys_result_t
save_pnm(char *path, image_data_t* src, pnm_data_type_t type);

sys_result_t
save_binary_pnm(char *path, image_data_t* src);

sys_result_t
save_ascii_pnm(char *path, image_data_t* src);

int can_open(const char* path);

static char *filters[] = {
	"*.ppm", "*.pgm", "*.pbm", "*.pnm"
};
static size_t nfilters = G_N_ELEMENTS(filters);

static plugin_load_handler_t pnm_load_handler;

static plugin_save_handler_t pnm_save_binary_handler;
static plugin_save_handler_t pnm_save_ascii_handler;

static plugin_t base_desc =
{
	"PNM extension plugin",
	"Milosz Kosobucki",
	"0.1",
	PLUGIN_FILEIO,
	pnm_plugin_load,
	pnm_plugin_unload,
};

static plugin_fileio_t io_plugin_desc;

plugin_t* clipt_plugin_info()
{
	//TODO: clean up this shit
	plugin_fileio_t *ret = malloc(sizeof(plugin_fileio_t));

	ret->base = base_desc;

	//Load Handlers
	ret->n_load_handlers = 1;
	ret->load_handlers = malloc(sizeof(plugin_load_handler_t *) * ret->n_load_handlers);

	pnm_load_handler.nfilters = nfilters;
	pnm_load_handler.filters = filters;
	pnm_load_handler.desc = "Portable Anymap (.ppm, .pgm, .pbm, .pnm)";
	pnm_load_handler.function = load_pnm;
	pnm_load_handler.can_open = can_open;

	ret->load_handlers[0] = &pnm_load_handler;

	//Save handlers
	ret->n_save_handlers = 2;
	ret->save_handlers = malloc(sizeof(plugin_save_handler_t *) * ret->n_save_handlers);

	pnm_save_ascii_handler.nfilters = nfilters;
	pnm_save_ascii_handler.filters = filters;
	pnm_save_ascii_handler.desc = "Portable Anymap (ASCII) (.ppm, .pgm, .pbm, .pnm)";
	pnm_save_ascii_handler.function = save_ascii_pnm;
	ret->save_handlers[0] = &pnm_save_ascii_handler;

	pnm_save_binary_handler.nfilters = nfilters;
	pnm_save_binary_handler.filters = filters;
	pnm_save_binary_handler.desc = "Portable Anymap (Raw) (.ppm, .pgm, .pbm, .pnm)";
	pnm_save_binary_handler.function = save_binary_pnm;
	ret->save_handlers[1] = &pnm_save_binary_handler;

	return (plugin_t*) ret;
}

sys_result_t pnm_plugin_load()
{
	io_plugin_desc.base = base_desc;

	return CLIPT_OK;
}

sys_result_t pnm_plugin_unload()
{
	return CLIPT_OK;
}

int can_open(const char* path)
{
	FILE *fd = fopen(path, "r");
	if(fd == NULL) {
		return 0;
	}
	char magic[2];
	fread(magic, sizeof(char), 2, fd);
	fclose(fd);

	// Checking for P1, P2, P3, P4, P5, P6
	if(
		magic[0] == 'P' &&
		(
			magic[1] == '1' ||
			magic[1] == '2' ||
			magic[1] == '3' ||
			magic[1] == '4' ||
			magic[1] == '5' ||
			magic[1] == '6'
		)
	) {
		return 1;
	} else {
		return 0;
	}

}

sys_result_t
save_binary_pnm(char *path, image_data_t* src)
{
	return save_pnm(path, src, PNM_BINARY);
}

sys_result_t
save_ascii_pnm(char *path, image_data_t* src)
{
	return save_pnm(path, src, PNM_ASCII);
}

/**
 *                                     (float) x * 255.0f
 * normalized(x,max) = (unsigned int) ------------------- + 0.5f
 *                                        (float) max
 */
#define normalize(x,max) (unsigned int)(((float)x*255.0f)/(float)max + 0.5f)

#define BYTE_TO_FLOAT(x) (float) x / 255.0f
#define FLOAT_TO_BYTE(x) (unsigned char) (x * 255.0f)

char* read_next_non_comment(FILE* fd) {

	char* line = NULL;
	size_t len = 0;
	do {
		getline(&line, &len, fd);
		if(line[0] == '#') {
			free(line);
			line = NULL;
		}
	} while (!line);
	return line;
}

/**
 * Read actual data from binary pnm file.
 * dst must have it's metadata properly filled before (height,
 * width, bpp), and allocated enough space for data.
 */
sys_result_t load_pnm_data(FILE* fd, pnm_data_type_t type, image_data_t* dst, int maxval);

sys_result_t load_pnm(const char *path, image_data_t **image)
{

	FILE *fd = fopen(path, "r");
	if(fd == NULL) {
		return CLIPT_ERROR;
	}
	char magic[2];
	fread(magic, sizeof(char), 2, fd);

	//Wrong magic
	if(magic[0] != 'P') {
		return CLIPT_ERROR;
	}
	//discarding newline character
	getc(fd);

	char* text_line = read_next_non_comment(fd);

	int width, height, maxval;
	sscanf(text_line, "%d %d", &width, &height);

	free(text_line);
	text_line = NULL;

	if(magic[1] != '1' && magic[1] != '4') {
		text_line = read_next_non_comment(fd);
		sscanf(text_line, "%d", &maxval);
		free(text_line);
	}
	image_data_t* ret = malloc(sizeof(image_data_t));

	if(!ret) {
		return CLIPT_ERESOURCES;
	}

	ret->width = width;
	ret->height = height;

	if(magic[1] == '1' || magic[1] == '2' || magic[1] == '4' || magic[1] == '5') {
		//Channels are always set to 3 because whole application works
		//only with rgb buffers
		ret->channels = 3;
	} else {
		ret->channels = 3;
	}

	ret->data = malloc(sizeof(float) * width * height * 3);

	int error;
	switch(magic[1]){
	case '1':
		ret->bpp = 1;
		error = load_pnm_data(fd, PNM_ASCII, ret, -1);
		break;
	case '2':
		ret->bpp = 8;
		error = load_pnm_data(fd, PNM_ASCII, ret, maxval);
		break;
	case '3':
		ret->bpp = 24;
		error = load_pnm_data(fd, PNM_ASCII, ret, maxval);
		break;
	case '4':
		ret->bpp = 1;
		error = load_pnm_data(fd, PNM_BINARY, ret, -1);
		break;
	case '5':
		ret->bpp = 8;
		error = load_pnm_data(fd, PNM_BINARY, ret, maxval);
		break;
	case '6':
		ret->bpp = 24;
		error = load_pnm_data(fd, PNM_BINARY, ret, maxval);
		break;
	}
	if(error != CLIPT_OK) {
		free(ret->data);
		free(ret);
		return CLIPT_ERROR;
	} else {
		*image = ret;
		return CLIPT_OK;
	}
}

sys_result_t load_bitmap(FILE* fp, image_data_t* dst)
{
	int num_bytes = (size_t) ceilf((float)(dst->width * dst->height) / 8.0f);
	int i,j;
	size_t data_idx = 0;
	unsigned char single_pixel;
	//last byte is probably padded, will be handled separately
	for(i=0; i<num_bytes - 1; i++) {
		unsigned char c = getc(fp);
		for(j=7; j>=0; j--) {
			single_pixel = (1 - ((c >> j) & 0x01)) * 255;
			dst->data[data_idx++] = BYTE_TO_FLOAT(single_pixel);
			dst->data[data_idx++] = BYTE_TO_FLOAT(single_pixel);
			dst->data[data_idx++] = BYTE_TO_FLOAT(single_pixel);
		}

	}

	//last byte
	unsigned char last_byte = getc(fp);
	size_t significant_bits = dst->width * dst->height - 8*(num_bytes - 1);
	for(i=significant_bits; i>=0; i--) {
		single_pixel = (1 - ((last_byte >> j) & 0x01)) * 255;
		dst->data[data_idx++] = BYTE_TO_FLOAT(single_pixel);
	}
	return CLIPT_OK;
}

sys_result_t load_pnm_data(FILE* fd, pnm_data_type_t type, image_data_t* dst, int maxval)
{

	// 1-bit binary data is handled separately
	if(type == PNM_BINARY && dst->bpp == 1) {
		return load_bitmap(fd, dst);
	}

	int i,j;
	int r,g,b;
	int lum;
	size_t dest_idx = 0;

	for(i=0; i < dst->height; i++) {
		for(j=0; j < dst->width; j++) {
			switch(dst->bpp) {
			case 1:
				// 1-bit binary data is handled separately in
				// load_bitmap(...)
				lum = (int) (getc(fd) - '0');
				lum = 1 - lum;
				dst->data[dest_idx++] = (float) lum;
				dst->data[dest_idx++] = (float) lum;
				dst->data[dest_idx++] = (float) lum;
				break;
			case 8:
				if(type == PNM_ASCII) {
					fscanf(fd, "%d", &lum);
				} else {
					lum = getc(fd);
				}
				unsigned char value = (unsigned char) normalize(lum, maxval);
				dst->data[dest_idx++] = BYTE_TO_FLOAT(value);
				dst->data[dest_idx++] = BYTE_TO_FLOAT(value);
				dst->data[dest_idx++] = BYTE_TO_FLOAT(value);
				break;
			case 24:
				if(type == PNM_ASCII) {
					fscanf(fd, "%d %d %d", &r, &g, &b);
				} else {
					r = getc(fd);
					g = getc(fd);
					b = getc(fd);
				}
				dst->data[dest_idx++] = BYTE_TO_FLOAT(normalize(r, maxval));
				dst->data[dest_idx++] = BYTE_TO_FLOAT(normalize(g, maxval));
				dst->data[dest_idx++] = BYTE_TO_FLOAT(normalize(b, maxval));
			}
		}
	}
	return CLIPT_OK;
}

static sys_result_t
save_pnm_data(FILE* fd, unsigned char data, pnm_data_type_t type, pnm_palette_t palette)
{
	static unsigned char _bits_buffer[2]; // thread unsafe. will go into loader context object in the future.
	unsigned char byte = data;

	if(type == PNM_ASCII)
		fprintf(fd, "%d ", data);
	else {
		if(palette == PNM_BITMAP) {
		  if(!data) _bits_buffer[0] += 1 << (7 - _bits_buffer[1]);
			if(++_bits_buffer[1] == 8) {
				byte = _bits_buffer[0];
				memset(_bits_buffer, 0, sizeof(_bits_buffer));
				fwrite(&byte, sizeof(unsigned char), 1, fd);
			}
		}
		else
			fwrite(&byte, sizeof(unsigned char), 1, fd);
	}
	return CLIPT_OK;
}

sys_result_t
save_pnm(char *path, image_data_t* src, pnm_data_type_t type)
{
	static char* _pnm_magic[]   = { "P1", "P2", "P3", "P4", "P5", "P6" };

	size_t ix, iy;
	float* ptr = src->data;
	unsigned char data[3];

	pnm_palette_t palette;
	switch(src->bpp) {
	case 1:
		palette = PNM_BITMAP;
		break;
	case 8:
		palette = PNM_GRAYSCALE;
		break;
	case 24:
	default:
		palette = PNM_RGB24;
		break;
	}

	FILE* fd = fopen(path, "w");
	if(fd == NULL) {
		return CLIPT_ERROR;
	}

	fprintf(fd, "%s\n%d %d\n", _pnm_magic[palette + 3*type], src->width, src->height);
	if(palette != PNM_BITMAP) fprintf(fd, "%d\n", 0xFF);

	for(iy=0; iy<src->height; iy++) {
		for(ix=0; ix<src->width; ix++) {
			switch(palette) {
			case PNM_BITMAP:
				data[0] = FLOAT_TO_BYTE(*ptr)?1:0;
				break;
			case PNM_GRAYSCALE:
				data[0] = FLOAT_TO_BYTE(*ptr);
				break;
			case PNM_RGB24:
				data[0] = FLOAT_TO_BYTE(*(ptr+0));
				data[1] = FLOAT_TO_BYTE(*(ptr+1));
				data[2] = FLOAT_TO_BYTE(*(ptr+2));
				break;
			}
			ptr += 3;

			save_pnm_data(fd, data[0], type, palette);
			if(palette == PNM_RGB24) {
				save_pnm_data(fd, data[1], type, palette);
				save_pnm_data(fd, data[2], type, palette);
			}
			if(type == PNM_ASCII && ix+1 < src->width)
				fputc(' ', fd);
		}
		if(type == PNM_ASCII) fputc('\n', fd);
	}
	fclose(fd);
	return CLIPT_OK;
}

