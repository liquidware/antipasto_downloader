//*******************************************************************************
//*  
//*   antipasto_downloader
//*   
//*   Written by Mark Sproul
//*  
//*******************************************************************************
//*   DESCRIPTION:
//*   Uses a simple packet / ack based protocol with checksum for antipasto_cores
//*   
//*  
//*******************************************************************************
//*  Detailed Edit history
//*  <MLS>  =  Mark Sproul, msproul@jove.rutgers.edu
//*  <CML>  =  Chris Ladden, avrman@gmail.com
//*******************************************************************************
//*	Jan 21,	2009	<CML> Formatted for github checkin and added version information
//*	Jan 15,	2009	<MLS> Converstation with Matt, I am going ro re-write the flash file system
//*******************************************************************************
//*  <MLS> Programming style
//*      never use 1 letter variable names, use LONG names,
//*      { and } always go on their own line
//*      never put 2 lines of code on the same line
//*      use TAB instead of multiple spaces
//*      a comment line of "****"s above the start of each routine
//*      leave a space before and after each operator "a + b" not "a+b"
//*    these next few rules have to be broken to keep compatibility
//*      routine names should start with a CAPITAL letter
//*      variable names should always start with lower case,
//*      global variables should start with a lower case "g" followed by an UPPER case char
//*      constants (#define ) should start with a lower case "k" followed by an UPPER case char
//*      underLine "_" is an ok char to use in variable names, routine names, and constants
//*      changes should include comments with your initials
//*******************************************************************************
//*******************************************************************************
//*  PREPROCESSOR DIRECTIVES
//*******************************************************************************

#define  kAntipastoDownloaderString  "antipasto_downloader"
#define  kAntipastoDownloaderVersion "V1.1.0"

#include	<stdio.h>	/* Standard input/output definitions */
#include	<string.h>	/* String function definitions */
#include	<unistd.h>	/* UNIX standard function definitions */
#include	<fcntl.h>	/* File control definitions */
#include	<errno.h>	/* Error number definitions */
#include	<termios.h> /* POSIX terminal control definitions */



#define IMAGE_INTERFACE_STORE		'S'
#define IMAGE_INTERFACE_READ		'R'
#define IMAGE_INTERFACE_INFO		'I'
#define IMAGE_INTERFACE_PAGE_DONE	'D'
#define IMAGE_INTERFACE_EXIT		'E'
#define IMAGE_INTERFACE_CHECKSUM	'C'
#define IMAGE_INTERFACE_ERASE		'R'


int		gUsbPortFD;				//*	File descriptor for the port
int		gTotalBlocksXmited	=	0;
int		gTotalErrors		=	0;
int		gVerboseMode		=	0;

void	SendOneFile(char *filePath);


//*******************************************************************************
//*	returns TRUE if OK
//*******************************************************************************
char	OpenUSBpath(char *usbFilePath)
{
char			openOK;
struct termios	options;
char			textMsg[80];


	gUsbPortFD	=	open(usbFilePath, O_RDWR | O_NOCTTY | O_NDELAY);
	if (gUsbPortFD == -1)
	{
		//*	Could not open the port.
		strcpy(textMsg, "Unable to open ");
		strcat(textMsg, usbFilePath);
		
		perror(textMsg);
		openOK	=	0;
	}
	else
	{
		fcntl(gUsbPortFD, F_SETFL, 0);

		//*	Get the current options for the port...
		tcgetattr(gUsbPortFD, &options);

//		cfsetispeed(&options, B115200);
//		cfsetospeed(&options, B115200);

		cfsetispeed(&options, B57600);
		cfsetospeed(&options, B57600);

		//*	Enable the receiver and set local mode...
		options.c_cflag	|=	(CLOCAL | CREAD);
		
		
		//*	Set the new options for the port...
		tcsetattr(gUsbPortFD, TCSANOW, &options);
		openOK	=	1;
	}
	return(openOK);
}

//*******************************************************************************
void	DumpData(char *devicePath)
{
char			textMsg[80];
int				ii;
int				readCount;
char			readBuffer[256];
char			openOK;

	openOK	=	OpenUSBpath(devicePath);
	if (openOK == 0)
	{
		//*	Could not open the port.
		strcpy(textMsg, "Unable to open ");
		strcat(textMsg, devicePath);
		
		perror(textMsg);
	}
	else
	{
		printf("OPEN OK!!!!\n");
		printf("###########################################\n");
		printf("###########################################\n");
	
#if 0
		write(gUsbPortFD, "I", 1);

		//*	wait a while until we get SOMETHING from the Slide
		readCount	=	0;
		while (readCount == 0)
		{
			readCount	=	read(gUsbPortFD, readBuffer, 80);
			if (readCount > 0)
			{
				readBuffer[readCount]	=	0;
				printf(readBuffer);
			}
		}
#endif
		printf("Sending INFO command\n");

		//*	now lets tell it to send us some data
		write(gUsbPortFD, "I", 1);

		write(gUsbPortFD, "E", 1);
		
		for (ii=0; ii<150; ii++)
		{
			if  (ii < 5)
			{
				write(gUsbPortFD, "I", 1);
			}
			else
			{
				write(gUsbPortFD, "E", 1);
			}
			//*	now lets read some data
			readCount	=	read(gUsbPortFD, readBuffer, 80);
			if (readCount > 0)
			{
				readBuffer[readCount]	=	0;
				printf(readBuffer);
			}
		}
	}


	printf("\n\n\nClosing port\n");
    close(gUsbPortFD);
	
}

char	ghelpmsg[]	=	
{  
	"\n"
	"Uses a simple packet / ack based protocol with checksum"
	" for antipasto_cores\n"
	"\n"
	"Usage\n"
	"    antipasto_downloader [-v] [/dev/yourport] filename    # downloads file to TouchSlide\n"
	"    antipasto_downloader [/dev/yourport]                  # Gets info from TouchSlide\n"
	"\n"
	"    Options\n"
	"          -v  verbose mode\n"

};


//*******************************************************************************
void	PrintHelp(void)
{
	printf(ghelpmsg);
}

//*******************************************************************************
main(int argc, char **argv)
{
char			usbDeviceName[80]	=	"/dev/tty.usbserial-A60061UH";
char			textMsg[80];
struct termios	options;
int				ii;
int				usbOpenOK;
int				readCount;
char			readBuffer[256];
   
	printf("\n\n"
          kAntipastoDownloaderString  "\n"
          kAntipastoDownloaderVersion "\n"
          "Written by Mark Sproul      \n"
          "\n\n");                           // <CML> Revised welcome message.
   
	//*	lets look at the args
	ii	=	1;
	while (ii < argc)
	{
		if (strncmp(argv[ii], "/dev", 4) == 0)
		{
			//*	we have a device path
			strcpy(usbDeviceName, argv[ii]);
		}
		else if (strncmp(argv[ii], "-h", 2) == 0)
		{
			//*	we have a device path
			PrintHelp();
			return;
		}
		else if (strncmp(argv[ii], "-v", 2) == 0)
		{
			//*	we have a device path
			gVerboseMode	=	1;
		}
		else
		{
			printf("%s\n", argv[ii]);
			usbOpenOK	=	OpenUSBpath(usbDeviceName);
			if (usbOpenOK)
			{
				write(gUsbPortFD, "S", 1);
				SendOneFile(argv[ii]);
			}
			else
			{
				printf("failed to open USB port\n");
			}
		}
		ii++;
	}

	if (argc <= 1)
	{
		DumpData(usbDeviceName);
	}
}

#define	kAscii_ACK 0x06
#define	kAscii_NAK 0x15

//*******************************************************************************
//*	returns 0 if OK, -1 if nak
int	GetAckNac(void)
{
char	ackBuffer[8];
int		readCount;
int		returnValue;

	returnValue		=	-1;
	ackBuffer[0]	=	0;
	readCount		=	read(gUsbPortFD, ackBuffer, 4);
	if (readCount < 1)
	{
		printf("Read failed to return any data\n");
	}
	else 
	{
		ackBuffer[readCount] =	0;
		printf("%02X-%s=", ackBuffer[0], &ackBuffer[1]);
		
		if (ackBuffer[0] == kAscii_ACK)
		{
			returnValue	=	0;
			printf("+");
		}
		else
		{
			printf("%02X-", ackBuffer[0]);
			gTotalErrors++;
		}
	
	}
	fflush(0);
	return(returnValue);
}

//*******************************************************************************
//*	this is for debugging purposes
//*******************************************************************************
void	DisplayHexData(char *xmitBlock, short blockLen)
{
short	ii;
short	cc;
char	lineBuff[128];
char	hexBuff[16];
char	asciiBuff[32];
short	theBute;


	cc			=	0;
	lineBuff[0]	=	0;
	for (ii=0; ii<blockLen; ii++)
	{
		theBute	=	xmitBlock[ii] & 0x00ff;
		
		sprintf(hexBuff, "%02X ", theBute);
		strcat(lineBuff, hexBuff);
		
		if ((theBute < 0x20) || (theBute >= 0x7f))
		{
			asciiBuff[cc]	=	'.';
		}
		else
		{
			asciiBuff[cc]	=	theBute;
		}
		cc++;
		if (cc >= 16)
		{
			asciiBuff[cc]	=	0;
			strcat(lineBuff, asciiBuff);
			strcat(lineBuff, "\n");
			printf(lineBuff);
			
			cc			=	0;
			lineBuff[0]	=	0;
		}
	}
	if (cc >= 0)
	{
		while (strlen(lineBuff) < 48)
		{
			strcat(lineBuff, " ");
		}
		asciiBuff[cc]	=	0;
		strcat(lineBuff, asciiBuff);
		strcat(lineBuff, "\n");
		printf(lineBuff);
		
		cc			=	0;
		lineBuff[0]	=	0;
	}
	printf("\n");
}


//*******************************************************************************
void	SendBuffer(char *dataBuffer, int buffSize)
{
int		ii;
char	myData[16];

	for (ii=0; ii<buffSize; ii++)
	{
		usleep(4700);

		myData[0]	=	dataBuffer[ii];
		
		write(gUsbPortFD, myData, 1);
		fflush(0);
		usleep(4700);
		
	}
	
}


#define	kXferBlockSize	512

//*******************************************************************************
//*	file transmission data
//*	
//*		len		value
//*		------------------------------
//*		1		SOH
//*		1		SOH					yes, 2 of them
//*		4		file size			in bytes
//*		12		filename			8.3  the_name.ext
//*		------------------------------
//*		17 total
//*******************************************************************************
void	SendFileHeader(char *fileName, long fileSize)
{
short	cc;
short	ii;
char	xmitBlock[kXferBlockSize + 100];
char	myFileName[128];
char	*slashptr;

	for (ii=0; ii<12; ii++)
	{
		myFileName[ii]	=	0;
	}



	cc	=	0;
	xmitBlock[cc++]	=	0x01;							//*	SOH (Start Of Header)
	xmitBlock[cc++]	=	0x01;							//*	SOH (Start Of Header)
	xmitBlock[cc++]	=	((fileSize >> 24) & 0x00ff);	//*	high byte of fileSize
	xmitBlock[cc++]	=	((fileSize >> 16) & 0x00ff);	//*	2nd byte of fileSize
	xmitBlock[cc++]	=	((fileSize >> 8) & 0x00ff);		//*	3rd byte of fileSize
	xmitBlock[cc++]	=	(fileSize & 0x00ff);			//*	low byte of fileSize

	strcpy(myFileName, fileName);
	//*	look for leading "." or "/"
	while ((myFileName[0] == '.') || (myFileName[0] == '/'))
	{
		strcpy(myFileName, &myFileName[1]);
	}

	//*	now look for any "/" in the path
	slashptr	=	strchr(myFileName, '/');
	while (slashptr != 0)
	{
		strcpy(myFileName, slashptr + 1);
		slashptr	=	strchr(myFileName, '/');
	}
	
	//*	now make sure its less than 12 chars
	while (strlen(myFileName) > 12)
	{
		//*	truncate the beginning
		strcpy(myFileName, &myFileName[1]);
	}

	//*	copy the name into the xmit buffer
	for (ii=0; ii<12; ii++)
	{
		xmitBlock[cc++]	=	myFileName[ii];
	}

	usleep(14000);
	SendBuffer(xmitBlock, 18);
	if (gVerboseMode)
	{
		DisplayHexData(xmitBlock, 18);
	}
}


//*******************************************************************************
//*	file transmission data
//*	
//*		len		value
//*		------------------------------
//*		1		STX
//*		2		block number		which block in the file
//*		512		data
//*		4		checksum			32 bit checksum
//*		------------------------------
//*		519 total
//*******************************************************************************
void	SendOneFile(char *filePath)
{
FILE	*myFILE;
char	inputFileBuffer[kXferBlockSize + 100];
char	xmitBlock[kXferBlockSize + 100];
size_t	readCount;
short	ii;
short	cc;
short	blockNumber;
short	blockCount;
long	checkSumValue;
long	fileSize;


	//*	open the file
	myFILE	=	fopen(filePath, "r");
	if (myFILE != 0)
	{
		//*	figure out the file size
		//*	first seek to the end of the file
		fseek(myFILE, 0, SEEK_END);
		fileSize	=	ftell(myFILE);
		rewind(myFILE);	//*	rewind back to the start

		blockCount	=	fileSize / 512;
		if ((fileSize & 512) > 0)
		{
			blockCount++;
		}
		printf("File = %s, size= %ld, blockCount=%d\n", filePath, fileSize, blockCount);

		SendFileHeader(filePath, fileSize);
		
		GetAckNac();
		
	
	
		readCount	=	1;
		blockNumber	=	0;
		while (readCount > 0)
		{
			//*	zero out the buffer
			for (ii=0; ii<kXferBlockSize; ii++)
			{
				inputFileBuffer[ii]	=	0;
				xmitBlock[ii]		=	0;
			}
			
			
			readCount	=	fread(inputFileBuffer, 1, kXferBlockSize, myFILE);
			if (readCount > 0)
			{
				cc	=	0;
				xmitBlock[cc++]	=	0x02;							//*	STX (Start Of TEXT)
				xmitBlock[cc++]	=	((blockNumber >> 8) & 0x00ff);	//*	high byte of block number
				xmitBlock[cc++]	=	(blockNumber & 0x00ff);			//*	low byte of block number
				
				
				//*	compute the checksum and copy it to the output buffer at the same time
				checkSumValue	=	0;
				for (ii=0; ii<kXferBlockSize; ii++)
				{
					checkSumValue	+=	(inputFileBuffer[ii] & 0x00ff);
					xmitBlock[cc++]	=	inputFileBuffer[ii];
				}
				//*	now put the checksum in the buffer
				xmitBlock[cc++]	=	((checkSumValue >> 24) & 0x00ff);	//*	high byte of checkSumValue
				xmitBlock[cc++]	=	((checkSumValue >> 16) & 0x00ff);	//*	2nd byte of checkSumValue
				xmitBlock[cc++]	=	((checkSumValue >> 8) & 0x00ff);	//*	3rd byte of checkSumValue
				xmitBlock[cc++]	=	(checkSumValue & 0x00ff);			//*	low byte of checkSumValue

				usleep(14000);
				SendBuffer(xmitBlock, cc);
				gTotalBlocksXmited++;
				GetAckNac();
				
				if (gVerboseMode)
				{
					DisplayHexData(xmitBlock, cc);
				}



				blockNumber++;
			}
			
		}
		printf("\n");
		printf("Total blocks transmitted %d\n",	gTotalBlocksXmited);
		printf("Total errors %d\n",				gTotalErrors);

		fclose(myFILE);
	}
}





