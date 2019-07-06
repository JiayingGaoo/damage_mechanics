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
#define MAXDimen 100
#define MAXDataNum 20000

#define FNET_ID1 0
//#define FNET_ID2 0	
//0:Sigmoid
//1:Relu
//2:tanh

//regular variables
int input_dimen;
int hidden_dimen;
int output_dimen;
double learn_rate;
int max_train_epoch;

//weights and bias
double weight_ih[MAXDimen][MAXDimen];
double weight_ho[MAXDimen][MAXDimen];
double bias_h[MAXDimen];
double bias_o[MAXDimen];

//train set data
double data_y_pre_in_one_epoch[MAXDataNum][MAXDimen];
double data_x_max[MAXDimen], data_x_min[MAXDimen];
double data_y_max[MAXDimen], data_y_min[MAXDimen];
int data_num;

//debug and printf
void InitConsoleWindow()
{
	AllocConsole();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle,_O_TEXT);
	FILE * hf = _fdopen( hCrt, "w" );
	*stdout = *hf;
}

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

	for(i=0;i<input_dimen;i++)
	{
		for(j=0;j<hidden_dimen;j++)
		{
			weight_ih[i][j] = 2.0*((double)rand()/(RAND_MAX))-1.0;
		}
	}

	//h->o
	for(i=0;i<hidden_dimen;i++)
	{
		for(j=0;j<output_dimen;j++)
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
	for (j=0;j<hidden_dimen;j++)
	{
		bias_h[j] = ((double)rand()/(RAND_MAX));
	}

	//bias in output layer
	for (k=0;k<output_dimen;k++)
	{
		bias_o[k] = ((double)rand()/(RAND_MAX));
	}
}

void InitNN()
{
	FILE *fp_read_train_data;
	char ori_data[40];
	CSingle_nnDlg* pdlg = (CSingle_nnDlg*)AfxGetApp()->m_pMainWnd;
	int i,k;

	//variables setting
	pdlg->UpdateData(TRUE);
	input_dimen = pdlg->m_input_dimen;
	hidden_dimen = pdlg->m_hidden_dimen;
	output_dimen = pdlg->m_output_dimen;
	learn_rate = pdlg->m_learn_rate;
	max_train_epoch = pdlg->m_max_train_epoch;

	//attemp to read one row data to give value for the max and min values for each dimen
	fp_read_train_data = fopen(pdlg->m_train_data_file_path, "r");
	for (i=0;i<input_dimen;i++)
	{
		fscanf(fp_read_train_data, "%s", ori_data);
		data_x_max[i] = atof(ori_data);
		data_x_min[i] = atof(ori_data);
	}
	for (k=0;k<output_dimen;k++)
	{
		fscanf(fp_read_train_data, "%s", ori_data);
		data_y_max[k] = atof(ori_data);
		data_y_min[k] = atof(ori_data);
	}
	fclose(fp_read_train_data);
}

void Data_Pre()
{
	FILE *fp_read_train_data;
	char ori_data[40];
	int i,k;
	CSingle_nnDlg* pdlg = (CSingle_nnDlg*)AfxGetApp()->m_pMainWnd;
	double data_temp;

	pdlg->UpdateData(TRUE);
	fp_read_train_data = fopen(pdlg->m_train_data_file_path, "r");
	data_num = 0;
	while(1)
	{
		for (i=0;i<input_dimen;i++)
		{
			fscanf(fp_read_train_data,"%s",ori_data);
			data_temp = atof(ori_data);
			if (data_temp>data_x_max[i])
			{
				data_x_max[i] = data_temp;
			}
			if (data_temp<data_x_min[i])
			{
				data_x_min[i] = data_temp;
			}
		}
		for (k=0;k<output_dimen;k++)
		{
			fscanf(fp_read_train_data,"%s",ori_data);
			data_temp = atof(ori_data);
			if (data_temp>data_y_max[k])
			{
				data_y_max[k] = data_temp;
			}
			if (data_temp<data_y_min[k])
			{
				data_y_min[k] = data_temp;
			}
		}
		data_num = data_num + 1;

		//printf("read_row_id = %d\n",data_num);
		if (feof(fp_read_train_data)||data_num>MAXDataNum)
		{
			printf("Complete reading data:\n%s: %d\n", pdlg->m_train_data_file_path,data_num);
			break;
		}
	}

//	//printf the min and max values for each dimension
//	for(i=0;i<input_dimen;i++)
//	{
//		printf("InputDimen:%d\n",i);
//		printf("max = %f, min = %f\n",data_x_max[i],data_x_min[i]);
//	}
//	for(k=0;k<output_dimen;k++)
//	{
//		printf("OutputDimen:%d\n",k);
//		printf("max = %f, min = %f\n",data_y_max[k],data_y_min[k]);
//	}
	//fscanf("%s",ori_data);
	fclose(fp_read_train_data);
}

void TrainBPNN()
{
	// TODO: Add your control notification handler code here
	FILE *fp_read_train_data;
	int epoch,i,j,k;
	char ori_data[40];
	double data_temp_x[MAXDimen], data_temp_y[MAXDimen];
	double t_y1[MAXDimen];
	double t_y2[MAXDimen];
	double tWeightDelta2[MAXDimen];
	double tWeightDelta1[MAXDimen];
	double t_sum;
	double accumulate_error;
	double sum_x[MAXDimen], sum_x2[MAXDimen], sum_y[MAXDimen], sum_y2[MAXDimen], sum_xy[MAXDimen], r2[MAXDimen];
	CSingle_nnDlg* pdlg = (CSingle_nnDlg*)AfxGetApp()->m_pMainWnd;
	
	InitNN();
	Data_Pre();
	WeightInit();
	BiasInit();
	
	pdlg->UpdateData(TRUE);
	//load training data and training:
	for(epoch = 0; epoch<max_train_epoch; epoch = epoch + 1)
	{
		fp_read_train_data = fopen(pdlg->m_train_data_file_path, "r");
		
		accumulate_error = 0.0;
		for (k=0;k<output_dimen;k++)
		{
			sum_x[k] = 0.0;
			sum_x2[k] = 0.0;
			sum_y[k] = 0.0;
			sum_y2[k] = 0.0;
			sum_xy[k] = 0.0;
		}
		
		//read each data in train set:
		while(1)
		{
			//read a piece of data and normalize it
			for (i=0;i<input_dimen;i++)
			{
				fscanf(fp_read_train_data, "%s", ori_data);
				data_temp_x[i] = atof(ori_data);
				//normalization
				data_temp_x[i] = (data_temp_x[i] - data_x_min[i])/(data_x_max[i] - data_x_min[i]);
			}
			for (k=0;k<output_dimen;k++)
			{
				fscanf(fp_read_train_data, "%s", ori_data);
				data_temp_y[k] = atof(ori_data);
				data_temp_y[k] = (data_temp_y[k] - data_y_min[k])/(data_y_max[k] - data_y_min[k]);
			}

			//Step1: foward calculation:
			//input layer->hidden layer
			for (j=0;j<hidden_dimen;j++)
			{
				t_sum = 0.0;
				for (i=0;i<input_dimen;i++)
				{
					t_sum = weight_ih[i][j]*data_temp_x[i] + t_sum;
				}
				t_y1[j] = fNet(t_sum + bias_h[j],FNET_ID1);
			}

			//hidden layer->output layer
			for (k=0;k<output_dimen;k++)
			{
				t_sum = 0.0;
				for (j=0;j<hidden_dimen;j++)
				{
					t_sum = weight_ho[j][k]*t_y1[j] + t_sum;
				}
				t_y2[k] = fNet(t_sum + bias_o[k],FNET_ID1);
				//the error accumulated as more data are reading

				accumulate_error = accumulate_error + fabs(data_temp_y[k] - t_y2[k]);
				sum_x[k] = sum_x[k] + t_y2[k];
				sum_x2[k] = sum_x2[k] + t_y2[k]*t_y2[k];
				sum_y[k] = sum_y[k] + data_temp_y[k];
				sum_y2[k] = sum_y2[k] + data_temp_y[k]*data_temp_y[k];
				sum_xy[k] = sum_xy[k] + t_y2[k]*data_temp_y[k];		
			}
			
			//Step 2. Backpropagation calculation
			//the tWeightDelta in output layer
			for (k=0;k<output_dimen;k++)
			{
				tWeightDelta2[k] = dNet(t_y2[k],FNET_ID1)*(data_temp_y[k]-t_y2[k]);
			}

			//adjust the weight from hidden to output layer		
			for (k=0;k<output_dimen;k++)
			{
				for (j=0;j<hidden_dimen;j++)
				{
					weight_ho[j][k] = weight_ho[j][k] + learn_rate*tWeightDelta2[k]*t_y1[j];
				}
				bias_o[k] = bias_o[k] + learn_rate*tWeightDelta2[k];
			}

			//adjust the weight from input to hidden layer
			for (j=0;j<hidden_dimen;j++)
			{
				t_sum = 0.0;
				for (k=0;k<output_dimen;k++)
				{
					t_sum = t_sum + tWeightDelta2[k]*weight_ho[j][k];
				}

				tWeightDelta1[j] = t_sum*dNet(t_y1[j], FNET_ID1);
			}

			for (j=0;j<hidden_dimen;j++)
			{
				for (i=0;i<input_dimen;i++)
				{
					weight_ih[i][j] = weight_ih[i][j] + learn_rate*tWeightDelta1[j]*data_temp_x[i];
				}
				bias_h[j] = bias_h[j] + learn_rate*tWeightDelta1[j];
			}			
			
			if (feof(fp_read_train_data))
			{
				for (k=0;k<output_dimen;k++)
				{
					r2[k] = ( sum_xy[k] - (sum_x[k]*sum_y[k])/((double)data_num) )/sqrt( (sum_x2[k] - (sum_x[k]*sum_x[k])/(((double)data_num)))*(sum_y2[k] - (sum_y[k]*sum_y[k])/(((double)data_num))) );
				}
				break;
			}
		}

		if (epoch%100==0)
		{
			printf("%d\t%f\t",epoch,accumulate_error);
			//printf regression cofficient for each output dimension
			for(k=0;k<output_dimen;k++)
			{
				printf("OutputDimen:%d\t",k);				
				printf("r2[%d] = %f\t",k,r2[k]);
			}
			printf("\n");
		}
		fclose(fp_read_train_data);
	}
}

void Test()
{
	FILE *fp_read_train_data;
	FILE* fp_test;
	int i,j,k;
	char ori_data[40];
	double t_y1[MAXDimen];
	double t_y2[MAXDimen];
	double data_temp_x[MAXDimen], data_temp_y[MAXDimen];
	double t_sum;
	CSingle_nnDlg* pdlg = (CSingle_nnDlg*)AfxGetApp()->m_pMainWnd;

	system("cls");
	fp_test = fopen("test_results.txt","w");
	fp_read_train_data = fopen(pdlg->m_train_data_file_path, "r");
	while(1)
	{
		//printf demension
		for (i=0;i<input_dimen;i++)
		{
			fscanf(fp_read_train_data,"%s",ori_data);
			data_temp_x[i] = atof(ori_data);
			fprintf(fp_test,"%f\t",data_temp_x[i]);
			//normalization:
			data_temp_x[i] = (data_temp_x[i] - data_x_min[i])/(data_x_max[i] - data_x_min[i]);
		}

		//input layer->hidden layer
		for (j=0;j<hidden_dimen;j++)
		{
			t_sum = 0.0;
			for (i=0;i<input_dimen;i++)
			{
				t_sum = weight_ih[i][j]*data_temp_x[i] + t_sum;
			}

			t_y1[j] = fNet(t_sum + bias_h[j],FNET_ID1);
		}

		//hidden layer->output layer
		for (k=0;k<output_dimen;k++)
		{
			fscanf(fp_read_train_data,"%s",ori_data);
			data_temp_y[k] = atof(ori_data);
			fprintf(fp_test,"%f\t", data_temp_y[k]);

			t_sum = 0.0;
			for (j=0;j<hidden_dimen;j++)
			{
				t_sum = weight_ho[j][k]*t_y1[j] + t_sum;
			}
			t_y2[k] = fNet(t_sum + bias_o[k],FNET_ID1);
			fprintf(fp_test,"%f", t_y2[k]*(data_y_max[k]-data_y_min[k]) + data_y_min[k]);
		}
		fprintf(fp_test, "\n");
		
		if (feof(fp_read_train_data))
		{
			break;
		}
	}
	fclose(fp_read_train_data);
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
	m_train_data_file_path = _T("TrainData_1.txt");
	m_input_dimen = 3;
	m_hidden_dimen = 30;
	m_output_dimen = 1;
	m_learn_rate = 0.2;
	m_activate_fun_id = 0;
	m_max_train_epoch = 10000;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSingle_nnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSingle_nnDlg)
	DDX_Text(pDX, IDC_EDIT1, m_train_data_file_path);
	DDX_Text(pDX, IDC_EDIT2, m_input_dimen);
	DDX_Text(pDX, IDC_EDIT3, m_hidden_dimen);
	DDX_Text(pDX, IDC_EDIT4, m_output_dimen);
	DDX_Text(pDX, IDC_EDIT5, m_learn_rate);
	DDX_Text(pDX, IDC_EDIT6, m_activate_fun_id);
	DDX_Text(pDX, IDC_EDIT7, m_max_train_epoch);
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
	InitNN();
	Data_Pre();
}

void CSingle_nnDlg::ReadTrainData()
{
}

void CSingle_nnDlg::OnTrain() 
{
	// TODO: Add your control notification handler code here
	Data_Pre();
	WeightInit();
	BiasInit();
	TrainBPNN();

}

void CSingle_nnDlg::OnTest() 
{
	// TODO: Add your control notification handler code here
	Test();	
}
