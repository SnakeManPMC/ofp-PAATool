// PAAToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PAATool.h"
#include "PAAToolDlg.h"
#include "PAAFile.h"

static char *mipmapFilterNames[14] = {
	"Point",    
	"Box",      
	"Triangle", 
	"Quadratic",
	"Cubic",    
	"Catrom",   
	"Mitchell", 
	"Gaussian", 
	"Sinc",     
	"Bessel",   
	"Hanning",  
	"Hamming",  
	"Blackman", 
	"Kaiser",
};

MIPFilterTypes mipmapFilterValues[14] = {
    kMIPFilterPoint,    
    kMIPFilterBox,      
    kMIPFilterTriangle, 
    kMIPFilterQuadratic,
    kMIPFilterCubic,    
    kMIPFilterCatrom,   
    kMIPFilterMitchell, 
    kMIPFilterGaussian, 
    kMIPFilterSinc,     
    kMIPFilterBessel,   
    kMIPFilterHanning,  
    kMIPFilterHamming,  
    kMIPFilterBlackman, 
    kMIPFilterKaiser,
};

static char *sharpenFilterNames[18] = {
	"None",
	"Negative",
	"Lighter",
	"Darker",
	"ContrastMore",
	"ContrastLess",
	"Smoothen",
	"SharpenSoft",
	"SharpenMedium",
	"SharpenStrong",
	"FindEdges",
	"Contour",
	"EdgeDetect",
	"EdgeDetectSoft",
	"Emboss",
	"MeanRemoval",
	"UnSharp",
	"XSharpen",
};

SharpenFilterTypes sharpenFilterValues[18] = {
    kSharpenFilterNone,
    kSharpenFilterNegative,
    kSharpenFilterLighter,
    kSharpenFilterDarker,
    kSharpenFilterContrastMore,
    kSharpenFilterContrastLess,
    kSharpenFilterSmoothen,
    kSharpenFilterSharpenSoft,
    kSharpenFilterSharpenMedium,
    kSharpenFilterSharpenStrong,
    kSharpenFilterFindEdges,
    kSharpenFilterContour,
    kSharpenFilterEdgeDetect,
    kSharpenFilterEdgeDetectSoft,
    kSharpenFilterEmboss,
    kSharpenFilterMeanRemoval,
    kSharpenFilterUnSharp,
    kSharpenFilterXSharpen,
};

#pragma pack(1)
typedef struct {
	char identsize;
	char colormaptype;
	char imagetype;

	short colormapstart;
	short colormaplength;
	char colormapbits;

	short xstart;
	short ystart;
	short width;
	short height;
	char bits;
	char descriptor;
} TGAHeader;
#pragma pack(8)

enum TextureType {
	txtUnknown,
	txtDXT1,
	txtDXT1a,
	txtDXT3,	
	txtDXT5,	
	txt4444,
	txt1555,
	txtIA88,
	txt8888,
	txt565,
	txtAutoDetect
};

enum TextureType textureTypeParams[6] = {
	txtAutoDetect,
	txtDXT1,
	txtDXT1a,
	txt1555,
	txt4444,
	txtIA88,
};

// globals for callback to use
PAAFile *callback_paa;
int callback_width;
int callback_height;

HRESULT mipmapCallback(void *data, int level, DWORD size) {
	int width = callback_width >> level;
	int height = callback_height >> level;

	if (callback_paa->type == 0xFF01 && width > 2 && height > 2) {
		PAAMipmap *mipmap = new PAAMipmap(callback_paa->type, width, height, (unsigned char *)data);
		callback_paa->addMipmap(mipmap);
	}
	if (callback_paa->type != 0xFF01 && width > 0 && height > 0) {
		PAAMipmap *mipmap = new PAAMipmap(callback_paa->type, width, height, (unsigned char *)data);
		callback_paa->addMipmap(mipmap);
	}

	return 0;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CPAAToolDlg dialog
CPAAToolDlg::CPAAToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPAAToolDlg::IDD, pParent)
	, m_textureFormat(0)
	, m_dithering(FALSE)
	, m_inputFilename(_T(""))
	, m_NoAlphaFiltering(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPAAToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_sourceFile);
	DDX_Control(pDX, IDC_LIST2, m_mipmapList);
	DDX_Control(pDX, IDC_LIST3, m_sharpenList);
	DDX_Radio(pDX, IDC_RADIO6, m_textureFormat);
	DDX_Check(pDX, IDC_CHECK1, m_dithering);
	DDX_Check(pDX, IDC_CHECK2, m_NoAlphaFiltering);
	DDX_Text(pDX, IDC_EDIT1, m_inputFilename);
}

BEGIN_MESSAGE_MAP(CPAAToolDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedConvert)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedSelect)
END_MESSAGE_MAP()


// CPAAToolDlg message handlers

BOOL CPAAToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	for (int i = 0; i < 14; i++) {
		m_mipmapList.AddString(mipmapFilterNames[i]);
	}
	m_mipmapList.SetCurSel(1);

	for (int i = 0; i < 18; i++) {
		m_sharpenList.AddString(sharpenFilterNames[i]);
	}
	m_sharpenList.SetCurSel(0);


	UpdateData(TRUE);
	m_textureFormat = 0;
	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPAAToolDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPAAToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPAAToolDlg::OnBnClickedSelect()
{
	CFileDialog filedialog(TRUE, ".tga", NULL, OFN_FILEMUSTEXIST, "TARGA Files(*.tga)|*.tga|All Files (*.*)|*.*||");
	if (filedialog.DoModal() == IDOK) {
		UpdateData(TRUE);
		m_inputFilename = filedialog.GetPathName();
		UpdateData(FALSE);
	}
}

enum TextureType CPAAToolDlg::AutodetectType(const unsigned char *data, const int size) {
	int alpha = 0;
	int bwimg = 1;

	for (int i = 0; i < size; i++) {
		if (data[i * 4 + 3] == 0) {
			alpha = 1;
		}
		if (data[i * 4 + 3] > 0 && data[i * 4 + 3] < 0xff) {
			alpha = 2;
			break;
		}
	}

	for (int i = 0; i < size; i++) {
		if (data[i * 4 + 0] != data[i * 4 + 1] || data[i * 4 + 1] != data[i * 4 + 2]) {
			bwimg = 0;
			break;
		}
	}

	if (bwimg == 1) return txtIA88;
	if (alpha == 0) return txtDXT1;
	if (alpha == 1) return txtDXT1a;
	if (alpha == 2) return txt4444;

	return txtDXT1;
}

void CPAAToolDlg::GetAverageColor(const unsigned char *data, const int size, unsigned char *dest) {
	int r, g, b, a;
	r = g = b = a = 0;
	for (int i = 0; i < size; i++) {
		r += data[i * 4 + 0];
		g += data[i * 4 + 1];
		b += data[i * 4 + 2];
		a += data[i * 4 + 3];
	}
	dest[0] = (unsigned char)(r / size);
	dest[1] = (unsigned char)(g / size);
	dest[2] = (unsigned char)(b / size);
	dest[3] = (unsigned char)(a / size);
}

int CPAAToolDlg::HasTransparency(const unsigned char *data, const int size) {
	for (int i = 0; i < size; i++)
		if (data[i * 4 + 3] != 0xFF) return 1;
	return 0;
}

void CPAAToolDlg::OnBnClickedConvert()
{
	UpdateData(TRUE);
	CString tmp;

	char s1[_MAX_DRIVE];
	char s2[_MAX_DIR];
	char s3[_MAX_FNAME];
	char default_output_filename[_MAX_PATH];

	_splitpath(m_inputFilename, s1, s2, s3, NULL);
	_makepath(default_output_filename, NULL, NULL, s3, "paa");
		
	CFileDialog filedialog(FALSE, ".paa", default_output_filename, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		"OFP Texture Files(*.paa;*.pac)|*.paa;*.pac|All Files (*.*)|*.*||");
	if (filedialog.DoModal() != IDOK) return;

	AfxGetApp()->DoWaitCursor(1); 

	CFile in, out;
	CFileException fileex;
	TGAHeader tgaHeader;
	unsigned char *image_buffer = NULL;

	//tmp.Format("Converting %s ->\n%s\n%s", m_inputFilename, filedialog.GetPathName(), default_output_filename);
	//MessageBox(tmp);

	if (in.Open(m_inputFilename, CFile::modeRead | CFile::shareDenyWrite, &fileex) == 0) {
		fileex.ReportError();
		return;
	}

	// read tga headers & check is they're valid
	if (in.Read(&tgaHeader, sizeof(tgaHeader)) != 18) {
		MessageBox("Could not read TGA header", NULL, MB_ICONERROR | MB_OK);
		return;
	}

	if (tgaHeader.bits != 32) {
		CString msg;
		msg.Format("TGA File is not 32bpp, was %d bpp", tgaHeader.bits);
		MessageBox(msg, NULL, MB_ICONERROR | MB_OK);
		return;
	}
	if (tgaHeader.colormaptype != 0) {
		MessageBox("ERROR: TGA file contains colormap, should have none", NULL, MB_ICONERROR | MB_OK);
		return;
	}
	if (tgaHeader.imagetype != 2 && tgaHeader.imagetype != 10) {
		MessageBox("TGA imagetype not truecolor or RLE compressed truecolor", NULL, MB_ICONERROR | MB_OK);
		return;
	}

	// skip possible extra headers
	in.Seek(tgaHeader.identsize, CFile::current);

	// allocate image buffer
	image_buffer = new unsigned char[tgaHeader.width * tgaHeader.height * 4];
	if (image_buffer == NULL) {
		MessageBox("Could not allocate image buffer, out of memory", NULL, MB_ICONERROR | MB_OK);
	}

	// input seems to be ok, now create output file
	if (out.Open(filedialog.GetPathName(), CFile::modeWrite | CFile::shareDenyWrite | CFile::modeCreate, &fileex) == FALSE) {
		fileex.ReportError();
		if (image_buffer != NULL) delete[] image_buffer;
		return;
	}

	// read image data
	if (tgaHeader.imagetype == 2) {
		// noncompressed
		in.Read(image_buffer, tgaHeader.width * tgaHeader.height * 4);
	}

	if (tgaHeader.imagetype == 10) {
		// RLE compressed
		int read_pixels = 0;
		unsigned int *buf_ptr = (unsigned int *)image_buffer;

		while (read_pixels < tgaHeader.width * tgaHeader.height) {
			unsigned char header = 0;
			int color, length;

			in.Read(&header, 1);
			length = (header & 0x7f) + 1;

			// repeat
			if (header & 0x80) {
				in.Read(&color, 4);
				for (int i = 0; i < length; i++) {
					*buf_ptr++ = color;
					read_pixels++;
				}
			} else {
				in.Read(buf_ptr, 4 * length);
				buf_ptr += length;
				read_pixels += length;
			}
		}
	}
	in.Close();

	// check out if image need top-down flipping
	if ((tgaHeader.descriptor & 0x20) == 0) {
		unsigned char *temp = new unsigned char[tgaHeader.width * tgaHeader.height * 4];
		if (temp == NULL) {
			MessageBox("Could not allocate flip buffer, out of memory", NULL, MB_ICONERROR | MB_OK);
			if (image_buffer != NULL) delete[] image_buffer;
			return;
		}
		for (int y = 0; y < tgaHeader.height; y++)
			memcpy(temp + (tgaHeader.height - y - 1) * tgaHeader.width * 4, 
				image_buffer + y * tgaHeader.width * 4, tgaHeader.width * 4);

		memcpy(image_buffer, temp, tgaHeader.width * tgaHeader.height * 4);
		delete[] temp;
	}

	// done, now output part
	PAAFile *paa = new PAAFile();

	// initialize static globals for callback to use
	callback_paa = paa;
	callback_width = tgaHeader.width;
	callback_height = tgaHeader.height;

	// setup PAA headers
	GetAverageColor(image_buffer, tgaHeader.width * tgaHeader.height, paa->avgcolor);
	paa->flag = HasTransparency(image_buffer, tgaHeader.width * tgaHeader.height);
	if (paa->flag && m_NoAlphaFiltering) paa->flag = 2;

	// setup NVIDIA library options
	CompressionOptions options;
	options.MIPFilterType = mipmapFilterValues[m_mipmapList.GetCurSel()];
	options.SharpenFilterType = sharpenFilterValues[m_sharpenList.GetCurSel()];

	options.sharpening_passes_per_mip_level[0] = 0;
	for (int s = 1; s < SHARP_TIMES_ENTRIES; s++)
		options.sharpening_passes_per_mip_level[s] = 1;

	// some funky expressions to wiggle around compiler warnings
	options.bDitherMIP0 = (m_dithering != 0);
	options.bDitherColor = (m_dithering != 0);

	enum TextureType type_param = textureTypeParams[m_textureFormat];

	if (type_param == txtAutoDetect) type_param = AutodetectType(image_buffer, tgaHeader.width * tgaHeader.height);

	if (type_param == txtDXT1) {
		paa->type = 0xFF01;
		options.TextureFormat = kDXT1;
	}
	if (type_param == txtDXT1a) {
		paa->type = 0xFF01;
		options.TextureFormat = kDXT1a;
	}
	if (type_param == txt1555) {
		paa->type = 0x1555;
		options.TextureFormat = k1555;
	}
	if (type_param == txtIA88) {
		paa->type = 0x8080;
		options.TextureFormat = kA8L8;
	}
	if (type_param == txt4444) {
		paa->type = 0x4444;
		options.TextureFormat = k4444;
	}

	nvDXTcompress(image_buffer, tgaHeader.width, tgaHeader.height, tgaHeader.width * 4, &options, 4, (MIPcallback)mipmapCallback, NULL);

	paa->writeFile(&out);
	out.Close();

	callback_paa = NULL;
	delete paa;

	if (image_buffer != NULL) delete[] image_buffer;

	AfxGetApp()->DoWaitCursor(-1); 
}

void WriteDTXnFile(unsigned long l, void *p) {
};

void ReadDTXnFile(unsigned long l, void *p) {
};