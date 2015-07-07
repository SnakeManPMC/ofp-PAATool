#ifndef __PAAFILE_H
#define __PAAFILE_H

class PAAMipmap {
public:
	int width;
	int height;
	int size;
	unsigned char *data;

	PAAMipmap(const int type, const int width, const int height, unsigned char *source);
	~PAAMipmap();
};
class PAAFile {
public:
	int type;
	int flag;
	int offsets[16];
	unsigned char avgcolor[4];
	int mipmap_num;

	PAAMipmap *mipmaps[16];

	PAAFile();
	~PAAFile();
	int writeFile(CFile *f);
	void addMipmap(PAAMipmap *mipmap);
};

#endif