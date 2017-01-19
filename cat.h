#ifndef CAT_H_INCLUDED
#define CAT_H_INCLUDED

typedef struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char	 pixel_data[64 * 64 * 2 + 1];
} image_t;

extern image_t cat_image;

#endif
