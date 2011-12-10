#include <stdio.h>
#include <math.h>
#include <string.h>
#include <config.h>
#include <system.h>
#include <image.h>
#include <plugin.h>

enum pnm_data_type_e {
        PNM_ASCII = 0,
        PNM_BINARY,
};
typedef enum pnm_data_type_e pnm_data_type_t;

sys_result_t pnm_plugin_load();
sys_result_t pnm_plugin_unload();

sys_result_t load_pnm(char *path, image_t **image);

plugin_t* clit_plugin_info()
{
       plugin_fileio_t *ret = malloc(sizeof(plugin_fileio_t));


       ret->base.name = "PNM extension plugin";
       ret->base.author = "Milosz Kosobucki";
       ret->base.version = "0.1";

       ret->base.type = PLUGIN_FILEIO;
       ret->base.plugin_load = pnm_plugin_load;
       ret->base.plugin_unload = pnm_plugin_unload;

       ret->n_load_handlers = 1;
       ret->load_handlers = malloc(sizeof(plugin_load_handler_t *) * 1);

       plugin_load_handler_t *handler = malloc(sizeof(plugin_load_handler_t));
       handler->ext = "pnm,pbm,pgm";
       handler->desc = "Portable Anymap";
       handler->function = load_pnm;

       ret->load_handlers[1] = handler;
       return (plugin_t*) ret;
}

sys_result_t pnm_plugin_load()
{
        return CLIT_OK;
}

sys_result_t pnm_plugin_unload()
{
        return CLIT_OK;
}

/**
 *                                 (float) y * 255.0f
 * normalized(x) = (unsigned int) ------------------- + 0.5f
 *                                    (float) max
 */
#define normalize(x,max) (unsigned int)(((float)x*255.0f)/(float)max + 0.5f)

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
sys_result_t load_pnm_data(FILE* fd, pnm_data_type_t type, image_t* dst, int maxval);

sys_result_t load_pnm(char *path, image_t **image)
{

        FILE *fd = fopen(path, "r");
        if(fd == NULL) {
                return CLIT_ERROR;
        }
        char magic[2];
        fread(magic, sizeof(char), 2, fd);

        //Wrong magic
        if(magic[0] != 'P') {
                return CLIT_ERROR;
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
        image_t* ret = malloc(sizeof(image_t));

        if(!ret) {
                return CLIT_ERESOURCES;
        }

        ret->width = width;
        ret->height = height;

        ret->data = malloc(sizeof(unsigned char) * width * height * 4);

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
        if(error != CLIT_OK) {
                free(ret->data);
                free(ret);
                return CLIT_ERROR;
        } else {
                *image = ret;
                return CLIT_OK;
        }
}

sys_result_t load_bitmap(FILE* fp, image_t* dst)
{
        size_t num_bytes = (size_t) ceilf((float)(dst->width * dst->height) / 8.0f);
        int i,j;
        size_t data_idx = 0;
        unsigned char single_pixel;
        //last byte is probably padded, will be handled separately
        for(i=0; i<num_bytes - 1; i++) {
                unsigned char c = getc(fp);
                for(j=7; j>=0; j--) {
                        single_pixel = (1 - ((c >> j) & 0x01)) * 255;
                        memset(dst->data + data_idx, single_pixel, 3);
                        data_idx += 3;
                        dst->data[data_idx++] = 0;
                }

        }

        //last byte
        unsigned char last_byte = getc(fp);
        size_t significant_bits = dst->width * dst->height - 8*(num_bytes - 1);
        for(i=significant_bits; i>=0; i--) {
                single_pixel = (1 - ((last_byte >> j) & 0x01)) * 255;
                memset(dst->data + data_idx, single_pixel, 3);
                data_idx += 3;
                dst->data[data_idx++] = 0;
        }
        return CLIT_OK;
}

sys_result_t load_pnm_data(FILE* fd, pnm_data_type_t type, image_t* dst, int maxval)
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
                                        dst->data[dest_idx++] = lum * 255;
                                        dst->data[dest_idx++] = lum * 255;
                                        dst->data[dest_idx++] = lum * 255;
                                        dst->data[dest_idx++] = 0;
                                        break;
                                case 8:
                                        if(type == PNM_ASCII) {
                                                fscanf(fd, "%d", &lum);
                                        } else {
                                                lum = getc(fd);
                                        }
                                        memset(dst->data + dest_idx, (unsigned char) normalize(lum, maxval), 3);
                                        dest_idx += 3;
                                        dst->data[dest_idx++] = 0;
                                        break;
                                case 24:
                                        if(type == PNM_ASCII) {
                                                fscanf(fd, "%d %d %d", &r, &g, &b);
                                        } else {
                                                r = getc(fd);
                                                g = getc(fd);
                                                b = getc(fd);
                                        }
                                        dst->data[dest_idx++] = normalize(b, maxval);
                                        dst->data[dest_idx++] = normalize(g, maxval);
                                        dst->data[dest_idx++] = normalize(r, maxval);
                                        dst->data[dest_idx++] = 0;
                        }
                }
        }
        return CLIT_OK;
}
