#include <windows.h>
#include "xvid.h"


#define XviD_DEC_ERR			-1		// �G���[�I��
#define XviD_DEC_END			0		// �f�[�^�W�J�I��
#define XviD_DEC_CONTINUE		1		// �f�[�^�W�J��
#define XviD_DEC_NEEDMORE		2		// �ǉ��f�[�^�ǂݍ��ݗv��


class XviDDec{
	BYTE	*mp4_buffer;
	BYTE	*mp4_ptr;
	BYTE	*out_buffer;
	void	*dec_handle;
	int		width;
	int		height;
	int		colorspace;
	DWORD	buffer_size;
	DWORD	totalsize;
	int		useful_bytes;
	int		filenr;
	BOOL	bInit;
	BOOL	bContinue;
	BOOL	forceSkip;
	BOOL	bFlash;

public:
	XviDDec(){
		ZeroMemory(this,sizeof(XviDDec));
	}
	~XviDDec(){
		Clear_XviD();
	}
	BOOL Start_XviD(int Xsize,int Ysize,int csp);	// ���C�u�����g�p�O�ɃR�[��
	void Clear_XviD(void);	// �t�@�C���Đ����I��������R�[��
	WORD GetVer(void);		// ���C�u�����̃o�[�W������Ԃ�
//	BOOL GetAviData(); // AVI�t�@�C���̏ڍ׃f�[�^���擾���܂��B
	int DecodeXviD(BYTE *inmemory, int inmemsize,
				int *rest, BYTE *outmemory,DWORD pitch);
	void ChangeColorSpace(int csp){	colorspace = csp; }
	void SetSkip(BOOL set){ forceSkip = set; }
	void SetInit(){ bInit = TRUE; }

protected:
/* decode one frame  */
	inline int dec_main(BYTE *istream,
		 BYTE *ostream,
		 int istream_size,
		 xvid_dec_stats_t *xvid_dec_stats,
		 DWORD pitch);
};
