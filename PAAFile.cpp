#include "stdafx.h"
#include "PAAFile.h"
#include "compress.h"

static char *tag_offset = "GGATSFFO";
static char *tag_avgcol = "GGATCGVA";
static char *tag_alpha = "GGATGALF";

PAAMipmap::PAAMipmap(const int mtype, const int mwidth, const int mheight, unsigned char *source) {
	width = mwidth;
	height = mheight;
	data = NULL;
	if (mtype == 0x1555 || mtype == 0x8080 || mtype == 0x4444) {
		data = new unsigned char[width * height * 3];
		size = compress(data, source, width * height * 2);
		return;
	}
	if (mtype == 0xFF01) {
		size = width * height / 2;
		data = new unsigned char[size];
		memcpy(data, source, size);
		return;
	}
}

PAAMipmap::~PAAMipmap() {
	if (data != NULL) delete[] data;
}

PAAFile::PAAFile() {
	mipmap_num = 0;
	type = 0x4444;
	flag = 0;
	for (int i = 0; i < 16; i++) {
		offsets[i] = 0;
		mipmaps[i] = NULL;
	}
	for (int i = 0; i < 4; i++)
		avgcolor[i] = 0xFF;
}

PAAFile::~PAAFile() {
	for (int i = 0; i < 16; i++)
		if (mipmaps[i] != NULL) delete mipmaps[i];
}

void PAAFile::addMipmap(PAAMipmap *mipmap) {
	if (mipmap_num > 15) return;
	mipmaps[mipmap_num] = mipmap;
	mipmap_num++;
}

int PAAFile::writeFile(CFile *f) {
	int tmp_i;
	ULONGLONG offsets_pos;

	// file type
	f->Write(&type, 2);

	// average color
	f->Write(tag_avgcol, 8);
	tmp_i = 4;
	f->Write(&tmp_i, 4);
	f->Write(avgcolor, 4);

	if (flag) {
		f->Write(tag_alpha, 8);
		tmp_i = 4;
		f->Write(&tmp_i, 4);
		f->Write(&flag, 4);
	}

	// offsets
	f->Write(tag_offset, 8);	
	tmp_i = 16 * 4;
	f->Write(&tmp_i, 4);

	offsets_pos = f->GetPosition();

	f->Write(offsets, 16 * 4);

	// end-of-headers marker
	tmp_i = 0;
	f->Write(&tmp_i, 2);

	// write mipmaps
	for (int i = 0; i < 16; i++) {
		if (mipmaps[i] != NULL) {
			offsets[i] = (int)f->GetPosition();
			f->Write(&mipmaps[i]->width, 2);
			f->Write(&mipmaps[i]->height, 2);
			f->Write(&mipmaps[i]->size, 3);
			f->Write(mipmaps[i]->data, mipmaps[i]->size);
		}
	}

	// end-of-mipmaps marker
	tmp_i = 0;
	f->Write(&tmp_i, 4);
	f->Write(&tmp_i, 2);

	// write offsets again
	f->Seek(offsets_pos, CFile::begin);
	f->Write(offsets, 16 * 4);

	return 0;
}

