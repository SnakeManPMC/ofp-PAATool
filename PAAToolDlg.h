// PAAToolDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <dxtlib.h>

// CPAAToolDlg dialog
class CPAAToolDlg : public CDialog
{
// Construction
public:
	CPAAToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PAATOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_sourceFile;
	CString m_inputFilename;
	CListBox m_mipmapList;
	CListBox m_sharpenList;
	int m_textureFormat;
	BOOL m_dithering;
	afx_msg void OnBnClickedSelect();
	afx_msg void OnBnClickedConvert();

	enum TextureType CPAAToolDlg::AutodetectType(const unsigned char *data, const int size);
	void CPAAToolDlg::GetAverageColor(const unsigned char *data, const int size, unsigned char *dest);
	int CPAAToolDlg::HasTransparency(const unsigned char *data, const int size);
	BOOL m_NoAlphaFiltering;
};
