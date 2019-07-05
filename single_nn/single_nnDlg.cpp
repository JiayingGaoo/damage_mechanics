// single_nnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "single_nn.h"
#include "single_nnDlg.h"
#include "math.h"
#include "stdlib.h"
#include <time.h> 
#include <io.h>  
#include <fcntl.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
#define InputDimen 6	
#define HiddenDimen 45
#define OutputDimen 1
#define LearningRate	0.2
#define MAXEpoch 10000
#define MAXDataNum 20000

#define FNET_ID1 0
//#define FNET_ID2 0	
//0:Sigmoid
//1:Relu
//2:tanh

//debug and printf
void InitConsoleWindow()
{
	AllocConsole();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle,_O_TEXT);
	FILE * hf = _fdopen( hCrt, "w" );
	*stdout = *hf;
}

//weights and bias
double weight_ih[InputDimen][HiddenDimen];
double weight_ho[HiddenDimen][OutputDimen];
double bias_h[HiddenDimen];
double bias_o[OutputDimen];

//train set data
double data_x[MAXDataNum][InputDimen], data_y[MAXDataNum][OutputDimen];
double data_y_pre_in_one_epoch[MAXDataNum][OutputDimen];
double data_x_max[InputDimen], data_x_min[InputDimen];
double data_y_max[OutputDimen], data_y_min[OutputDimen];
int data_num;

//Dialog point
CSingle_nnDlg* p_single_nn_dlg;

double fNet(double tx, int fid)
{
	if (fid==0)
	{//Sigmoid
		return 1.0/(1.0 + exp(-tx));
	}
	if (fid==1)
	{//relu
		if (tx>0.0)
		{
			return tx;
		}
		else
		{
			return 0.0;
		}
	}
	if (fid==2)
	{
		return (exp(tx) - exp(-tx))/(exp(tx) + exp(-tx));
	}
	return 0.0;
}

double dNet(double ty, int fid)
{
	if (fid==0)
	{//Sigmoid
		return ty * (1.0 - ty);
	}
	if (fid==1)
	{//relu
		if (ty>0.0001)
		{
			return 1.0;
		}
		if (ty<0.0001)
		{
			return 0.0;
		}
	}
	if (fid==2)
	{//tanh
		return (1.0 - ty)*(1.0 + ty);
	}
	return 0.0;
}

void WeightInit()
{
	int i,j;
	//init weight values in (0,1)
	srand((unsigned)time(NULL)); 

	for(i=0;i<InputDimen;i++)
	{
		for(j=0;j<HiddenDimen;j++)
		{
			weight_ih[i][j] = 2.0*((double)rand()/(RAND_MAX))-1.0;
		}
	}

	//h->o
	for(i=0;i<HiddenDimen;i++)
	{
		for(j=0;j<OutputDimen;j++)
		{
			weight_ho[i][j] = 2.0*((double)rand()/(RAND_MAX))-1.0;
		}
	}
}

void BiasInit()
{
	int j,k;
	srand((unsigned)time(NULL)); 

	//bias in hidden layer
	for (j=0;j<HiddenDimen;j++)
	{
		bias_h[j] = ((double)rand()/(RAND_MAX));
	}

	//bias in output layer
	for (k=0;k<OutputDimen;k++)
	{
		bias_o[k] = ((double)rand()/(RAND_MAX));
	}
}

void LoadTrainData()
{
	FILE *fp_read_train_data;
	char ori_data[40];
	int i;
	p_single_nn_dlg->UpdateData(TRUE);
	fp_read_train_data = fopen(p_single_nn_dlg->m_train_data_file_path, "r");
	data_num = 0;
	while(1)
	{
		for (i=0;i<InputDimen;i++)
		{
			fscanf(fp_read_train_data,"%s",ori_data);
			data_x[data_num][i] = atof(ori_data);
		}
		for (i=0;i<OutputDimen;i++)
		{
			fscanf(fp_read_train_data,"%s",ori_data);
			data_y[data_num][i] = atof(ori_data);
		}
		data_num = data_num + 1;

		printf("read_row_id = %d\n",data_num);
		if (feof(fp_read_train_data)||data_num>MAXDataNum)
		{
			printf("Complete data read, row = %d\n", data_num);
			break;
		}
	}
	//fscanf("%s",ori_data);
	fclose(fp_read_train_data);
}

void Normalization()
{
	int i,j,k,n;
	double t_max, t_min;
	//obtain the max and min value for each dimension
	for(i = 0;i<InputDimen;i++)
	{
		t_max = data_x[i][0];
		t_min = data_x[i][0];
		for (n = 1;n<data_num;n++)
		{
			if(data_x[n][i]>t_max)
			{
				t_max = data_x[n][i];
			}
			if(data_x[n][i]<t_min)
			{
				t_min = data_x[n][i];
			}
		}
		data_x_max[i] = t_max;
		data_x_min[i] = t_min;
	}
	for(k=0;k<OutputDimen;k++)
	{
		t_max = data_y[k][0];
		t_min = data_y[k][0];
		for(n=1;n<data_num;n++)
		{
			if(data_y[n][k]>t_max)
			{
				t_max = data_y[n][k];
			}
			if(data_y[n][k]<t_min)
			{
				t_min = data_y[n][k];
			}
		}
		data_y_max[k] = t_max;
		data_y_min[k] = t_min;
	}
	//printf
	for(i=0;i<InputDimen;i++)
	{
		printf("InputDimen:%d\n",i);
		printf("max = %f, min = %f\n",data_x_max[i],data_x_min[i]);
	}
	for(k=0;k<OutputDimen;k++)
	{
		printf("OutputDimen:%d\n",k);
		printf("max = %f, min = %f\n",data_y_max[k],data_y_min[k]);
	}

	//nornalization:
	for(n=0;n<data_num;n++)
	{
		for(i=0;i<InputDimen;i++)
		{
			data_x[n][i] = (data_x[n][i] - data_x_min[i])/(data_x_max[i] - data_x_min[i]);
		}
		for(k=0;k<OutputDimen;k++)
		{
			data_y[n][k] = (data_y[n][k] - data_y_min[k])/(data_y_max[k] - data_y_min[k]);
		}
	}
}

void TrainBPNN()
{
	// TODO: Add your control notification handler code here
	int epoch,i,j,k,n;
	double t_y1[HiddenDimen];
	double t_y2[OutputDimen];
	double tWeightDelta2[OutputDimen];
	double tWeightDelta1[HiddenDimen];
	double t_sum;
	double accumulate_error;
	double sum_x, sum_x2, sum_y, sum_y2, sum_xy, r2;

	LoadTrainData();
	Normalization();
	WeightInit();
	BiasInit();

	//load training data and training:
	for(epoch = 0; epoch<MAXEpoch; epoch = epoch + 1)
	{
		accumulate_error = 0.0;
		for (n=0;n<data_num;n++)
		{
			//In this for(), the aim is to calculate the actual output of corresponding to data_y[n]
			
			//Step1: foward calculation:
			//input layer->hidden layer
			for (j=0;j<HiddenDimen;j++)
			{
				t_sum = 0.0;
				for (i=0;i<InputDimen;i++)
				{
					t_sum = weight_ih[i][j]*data_x[n][i] + t_sum;
				}

				t_y1[j] = fNet(t_sum + bias_h[j],FNET_ID1);
			}

			//hidden layer->output layer
			for (k=0;k<OutputDimen;k++)
			{
				t_sum = 0.0;
				for (j=0;j<HiddenDimen;j++)
				{
					t_sum = weight_ho[j][k]*t_y1[j] + t_sum;
				}
				t_y2[k] = fNet(t_sum + bias_o[k],FNET_ID1);
				data_y_pre_in_one_epoch[n][k] = t_y2[k];
				//the error accumulated as more data are reading
				accumulate_error = accumulate_error + fabs(data_y[n][k] - t_y2[k]);
			}
			
			//Step 2. Backpropagation calculation
			//the tWeightDelta in output layer
			for (k=0;k<OutputDimen;k++)
			{
				tWeightDelta2[k] = dNet(t_y2[k],FNET_ID1)*(data_y[n][k]-t_y2[k]);
			}

			//adjust the weight from hidden to output layer		
			for (k=0;k<OutputDimen;k++)
			{
				for (j=0;j<HiddenDimen;j++)
				{
					weight_ho[j][k] = weight_ho[j][k] + LearningRate*tWeightDelta2[k]*t_y1[j];
				}
				bias_o[k] = bias_o[k] + LearningRate*tWeightDelta2[k];
			}

			//adjust the weight from input to hidden layer
			for (j=0;j<HiddenDimen;j++)
			{
				t_sum = 0.0;
				for (k=0;k<OutputDimen;k++)
				{
					t_sum = t_sum + tWeightDelta2[k]*weight_ho[j][k];
				}

				tWeightDelta1[j] = t_sum*dNet(t_y1[j], FNET_ID1);
			}

			for (j=0;j<HiddenDimen;j++)
			{
				for (i=0;i<InputDimen;i++)
				{
					weight_ih[i][j] = weight_ih[i][j] + LearningRate*tWeightDelta1[j]*data_x[n][i];
				}
				bias_h[j] = bias_h[j] + LearningRate*tWeightDelta1[j];
			}			
		}
		if (epoch%100==0)
		{
			printf("%d\t%f\n",epoch,accumulate_error);
			//calculate regression cofficient for each output dimension
			//_x is predicted value, _y is label value
			for(k=0;k<OutputDimen;k++)
			{
				printf("OutputDimen:%d\n",k);
				sum_x = 0.0;
				sum_x2 = 0.0;
				sum_y = 0.0;
				sum_y2 = 0.0;
				sum_xy = 0.0;
				for(n=0;n<data_num;n++)
				{
					sum_x = sum_x + data_y_pre_in_one_epoch[n][k];
					sum_x2 = sum_x2 + data_y_pre_in_one_epoch[n][k]*data_y_pre_in_one_epoch[n][k];
					sum_y = sum_y + data_y[n][k];
					sum_y2 = sum_y2 + data_y[n][k]*data_y[n][k];
					sum_xy = sum_xy + data_y_pre_in_one_epoch[n][k]*data_y[n][k];
				}
				r2 = ( sum_xy - (sum_x*sum_y)/((double)data_num) )/sqrt( (sum_x2 - (sum_x*sum_x)/(((double)data_num)))*(sum_y2 - (sum_y*sum_y)/(((double)data_num))) );
				printf("r2 = %f\n",r2);
			}
		}
	}
}

void Test()
{
	FILE* fp_test;
	int i,j,k,n;
	double t_y1[HiddenDimen];
	double t_y2[OutputDimen];
	double t_sum;

	system("cls");
	fp_test = fopen("test_results.txt","w");
	for(n=0;n<data_num;n++)
	{
		//printf demension
		for (i=0;i<InputDimen;i++)
		{
			fprintf(fp_test,"%f\t",data_x[n][i]*(data_x_max[i] - data_x_min[i]) + data_x_min[i]);
		}

		//input layer->hidden layer
		for (j=0;j<HiddenDimen;j++)
		{
			t_sum = 0.0;
			for (i=0;i<InputDimen;i++)
			{
				t_sum = weight_ih[i][j]*data_x[n][i] + t_sum;
			}

			t_y1[j] = fNet(t_sum + bias_h[j],FNET_ID1);
		}

		//hidden layer->output layer
		for (k=0;k<OutputDimen;k++)
		{
			t_sum = 0.0;
			for (j=0;j<HiddenDimen;j++)
			{
				t_sum = weight_ho[j][k]*t_y1[j] + t_sum;
			}
			t_y2[k] = fNet(t_sum + bias_o[k],FNET_ID1);
			
			fprintf(fp_test,"%f\t", data_y[n][k]*(data_y_max[k]-data_y_min[k]) + data_y_min[k]);
			fprintf(fp_test,"%f", t_y2[k]*(data_y_max[k]-data_y_min[k]) + data_y_min[k]);
		}
		fprintf(fp_test, "\n");
	}
	fclose(fp_test);
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSingle_nnDlg dialog

CSingle_nnDlg::CSingle_nnDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSingle_nnDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSingle_nnDlg)
	m_train_data_file_path = _T("TrainData_zhanjiba.txt");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSingle_nnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSingle_nnDlg)
	DDX_Text(pDX, IDC_EDIT1, m_train_data_file_path);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSingle_nnDlg, CDialog)
	//{{AFX_MSG_MAP(CSingle_nnDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON2, OnLoadData)
	ON_BN_CLICKED(IDC_BUTTON1, OnTrain)
	ON_BN_CLICKED(IDC_BUTTON3, OnTest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSingle_nnDlg message handlers

BOOL CSingle_nnDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	InitConsoleWindow();
	p_single_nn_dlg = this;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSingle_nnDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSingle_nnDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSingle_nnDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSingle_nnDlg::OnLoadData() 
{
	// TODO: Add your control notification handler code here
	LoadTrainData();
	Normalization();
}

void CSingle_nnDlg::ReadTrainData()
{
}

void CSingle_nnDlg::OnTrain() 
{
	// TODO: Add your control notification handler code here
	LoadTrainData();
	WeightInit();
	BiasInit();
	TrainBPNN();

}

void CSingle_nnDlg::OnTest() 
{
	// TODO: Add your control notification handler code here
	Test();	
}
