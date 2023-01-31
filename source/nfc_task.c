/*
*         Copyright (c), NXP Semiconductors Caen / France
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <tool.h>
#include <Nfc.h>
#include <ndef_helper.h>
#include "fsl_sim.h"
#include "fsl_flash.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "tml.h"
#include "board.h"
#include "pin_mux.h"
#include "queue.h"

#define print_buf(x,y,z)  {int loop; printf(x); for(loop=0;loop<z;loop++) printf("0x%.2x ", y[loop]); printf("\n");}
#define app_run	1
//#define fabrica 1
unsigned char cont_erro = 0, cont_erro2 = 0, cont_erro3 = 0;
//codigo da flash
/*******************************************************************************
 * Definitions
 ******************************************************************************/


#define BUFFER_LEN 4

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void error_trap(void);
void app_finalize(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Flash driver Structure */
flash_config_t s_flashDriver;
/*! @brief Buffer for program */
static uint32_t s_buffer[BUFFER_LEN];
/*! @brief Buffer for readback */
static uint32_t s_buffer_rbc[BUFFER_LEN];

ftfx_security_state_t securityStatus = 'kFTFx_SecurityStateNotSecure'; /* Return protection status */
status_t result;    /* Return code from each flash driver function */
uint32_t destAdrss; /* Address of the target location */
uint32_t i, failAddr, failDat;

uint32_t pflashBlockBase = 0;
uint32_t pflashTotalSize = 0;
uint32_t pflashSectorSize = 0;

extern QueueHandle_t fila_mux;
extern QueueHandle_t fila_tripa;
/*
* @brief Gets called when an error occurs.
*
* @details Print error message and trap forever.
*/
void error_trap(void)
{
    printf("\r\n\r\n\r\n\t---- HALTED DUE TO FLASH ERROR! ----");
    while (1)
    {
    }
}

/*
* @brief Gets called when the app is complete.
*
* @details Print finshed message and trap forever.
*/
void app_finalize(void)
{
    /* Print finished message. */
    printf("\r\n End of PFlash Example \r\n");
    while (1)
    {
    }
}

//codigo da flash

/// pina
unsigned char acesso=0; //acesso não permitido
unsigned int tempo;
unsigned int exames;
unsigned char exame_feito;
float volume;
unsigned int locked = 0;

/* Discovery loop configuration according to the targeted modes of operation */
unsigned char DiscoveryTechnologies[] = {
#if defined P2P_SUPPORT || defined RW_SUPPORT
	MODE_POLL | TECH_PASSIVE_NFCA,
	MODE_POLL | TECH_PASSIVE_NFCF,
#endif
#ifdef RW_SUPPORT
	MODE_POLL | TECH_PASSIVE_NFCB,
	MODE_POLL | TECH_PASSIVE_15693,
#endif
#ifdef P2P_SUPPORT
	MODE_POLL | TECH_ACTIVE_NFCF,
#endif
#if defined P2P_SUPPORT || defined CARDEMU_SUPPORT
	MODE_LISTEN | TECH_PASSIVE_NFCA,
#endif
#if defined CARDEMU_SUPPORT
	MODE_LISTEN | TECH_PASSIVE_NFCB,
#endif
#ifdef P2P_SUPPORT
	MODE_LISTEN | TECH_PASSIVE_NFCF,
	MODE_LISTEN | TECH_ACTIVE_NFCA,
	MODE_LISTEN | TECH_ACTIVE_NFCF,
#endif
};

/* Mode configuration according to the targeted modes of operation */
unsigned mode = 0
#ifdef CARDEMU_SUPPORT
			  | NXPNCI_MODE_CARDEMU
#endif
#ifdef P2P_SUPPORT
			  | NXPNCI_MODE_P2P
#endif
#ifdef RW_SUPPORT
			  | NXPNCI_MODE_RW
#endif
;

#if defined P2P_SUPPORT || defined RW_SUPPORT
void NdefPull_Cb(unsigned char *pNdefMessage, unsigned short NdefMessageSize)
{
	unsigned char *pNdefRecord = pNdefMessage;
	NdefRecord_t NdefRecord;
	unsigned char save;

	if (pNdefMessage == NULL)
	{
		printf("--- Issue during NDEF message reception (check provisioned buffer size) \n");
		return;
	}

	while (pNdefRecord != NULL)
	{
		printf("--- NDEF record received:\n");

		NdefRecord = DetectNdefRecordType(pNdefRecord);

		switch(NdefRecord.recordType)
		{
		case MEDIA_VCARD:
			{
				save = NdefRecord.recordPayload[NdefRecord.recordPayloadSize];
				NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = '\0';
				printf("   vCard:\n%s\n", NdefRecord.recordPayload);
				NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = save;
			}
			break;

		case WELL_KNOWN_SIMPLE_TEXT:
			{
				save = NdefRecord.recordPayload[NdefRecord.recordPayloadSize];
				NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = '\0';
				printf("   Text record: %s\n", &NdefRecord.recordPayload[NdefRecord.recordPayload[0]+1]);
				NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = save;
			}
			break;

		case WELL_KNOWN_SIMPLE_URI:
			{
				save = NdefRecord.recordPayload[NdefRecord.recordPayloadSize];
				NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = '\0';
				printf("   URI record: %s%s\n", ndef_helper_UriHead(NdefRecord.recordPayload[0]), &NdefRecord.recordPayload[1]);
				NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = save;
			}
			break;

		case MEDIA_HANDOVER_WIFI:
			{
				unsigned char index = 26, i;

				printf ("--- Received WIFI credentials:\n");
				if ((pNdefRecord[index] == 0x10) && (pNdefRecord[index+1] == 0x0e)) index+= 4;
				while(index < NdefRecord.recordPayloadSize)
				{
					if (pNdefRecord[index] == 0x10)
					{
						if (pNdefRecord[index+1] == 0x45) {printf ("- SSID = "); for(i=0;i<pNdefRecord[index+3];i++) printf("%c", pNdefRecord[index+4+i]); printf ("\n");}
						else if (pNdefRecord[index+1] == 0x03) printf ("- Authenticate Type = %s\n", ndef_helper_WifiAuth(pNdefRecord[index+5]));
						else if (pNdefRecord[index+1] == 0x0f) printf ("- Encryption Type = %s\n", ndef_helper_WifiEnc(pNdefRecord[index+5]));
						else if (pNdefRecord[index+1] == 0x27) {printf ("- Network key = "); for(i=0;i<pNdefRecord[index+3];i++) printf("#"); printf ("\n");}
						index += 4 + pNdefRecord[index+3];
					}
					else continue;
				}
			}
			break;

		default:
			printf("   Unsupported NDEF record, cannot parse\n");
			break;
		}
		pNdefRecord = GetNextRecord(pNdefRecord);
	}

	printf("\n");
}
#endif

#if defined P2P_SUPPORT || defined CARDEMU_SUPPORT
const char NDEF_MESSAGE[] = { 0xD1,   // MB/ME/CF/1/IL/TNF
		0x01,   // TYPE LENGTH
		0x07,   // PAYLOAD LENTGH
		'T',    // TYPE
		0x02,   // Status
		'e', 'n', // Language
		'T', 'e', 's', 't' };

void NdefPush_Cb(unsigned char *pNdefRecord, unsigned short NdefRecordSize) {
	printf("--- NDEF Record sent\n\n");
}
#endif

#if defined RW_SUPPORT
#ifdef RW_RAW_EXCHANGE
void Keep_Configuration( void ){
	tml_Connect ();
	NxpNci_KeepConfiguration();
	if (NxpNci_StartDiscovery(DiscoveryTechnologies,sizeof(DiscoveryTechnologies)) != NFC_SUCCESS)
	{
		printf("Error: cannot start discovery\n");
		return;
	}

}

void MIFARE_scenario (void)
{
	bool status;
	unsigned char i;
	unsigned char Resp[256];
	unsigned char RespSize;
	/* Authenticate sector 1 with generic keys */
	unsigned char Auth[] = {0x40, 0x01, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	/* Read block 4 */
	unsigned char Read[] = {0x10, 0x30, 0x04};
	/* Write block 4 */
	unsigned char WritePart1[] = {0x10, 0xA0, 0x04};
	unsigned char WritePart2[] = {0x10, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

	/* Authenticate */
	status = NxpNci_ReaderTagCmd(Auth, sizeof(Auth), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Authenticate sector %d failed with error 0x%02x\n", Auth[1], Resp[RespSize-1]);
		return;
	}
	printf(" Authenticate sector %d succeed\n", Auth[1]);

	/* Read block */
	status = NxpNci_ReaderTagCmd(Read, sizeof(Read), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read block %d failed with error 0x%02x\n", Read[2], Resp[RespSize-1]);
		return;
	}
	printf(" Read block %d: ", Read[2]); for(i=0;i<RespSize-2;i++) printf("0x%02X ", Resp[i+1]); printf("\n");

	/* Write block */
	status = NxpNci_ReaderTagCmd(WritePart1, sizeof(WritePart1), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", WritePart1[2], Resp[RespSize-1]);
		return;
	}
	status = NxpNci_ReaderTagCmd(WritePart2, sizeof(WritePart2), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", WritePart1[2], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", WritePart1[2]);

	/* Read block */
	status = NxpNci_ReaderTagCmd(Read, sizeof(Read), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read failed with error 0x%02x\n", Resp[RespSize-1]);
		return;
	}
	printf(" Read block %d: ", Read[2]); for(i=0;i<RespSize-2;i++) printf("0x%02X ", Resp[i+1]); printf("\n");
}


void ISO15693_scenario (void)
{
	bool status;
	unsigned char i;
	unsigned char Resp[256];
	unsigned char RespSize;
	unsigned char ReadBlock[] = {0x02, 0x20, 0x08};
	unsigned char WriteBlock[] = {0x02, 0x21, 0x08, 0x11, 0x22, 0x33, 0x44};

	status = NxpNci_ReaderTagCmd(ReadBlock, sizeof(ReadBlock), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0x00))
	{
		printf(" Read block %d failed with error 0x%02x\n", ReadBlock[2], Resp[RespSize-1]);
		return;
	}
	printf(" Read block %d: ", ReadBlock[2]); for(i=0;i<RespSize-2;i++) printf("0x%02X ", Resp[i+1]); printf("\n");

	/* Write */
	status = NxpNci_ReaderTagCmd(WriteBlock, sizeof(WriteBlock), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", WriteBlock[2], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", WriteBlock[2]);

	/* Read back */
	status = NxpNci_ReaderTagCmd(ReadBlock, sizeof(ReadBlock), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0x00))
	{
		printf(" Read block %d failed with error 0x%02x\n", ReadBlock[2], Resp[RespSize-1]);
		return;
	}
	printf(" Read block %d: ", ReadBlock[2]); for(i=0;i<RespSize-2;i++) printf("0x%02X ", Resp[i+1]); printf("\n");
}


void Preparacao_fabrica (NxpNci_RfIntf_t RfIntf)
{
	bool status;
	unsigned char i;
	unsigned char Resp[256];
	unsigned char RespSize;
	/* Read block 4 */
	unsigned char Read[] = {0x30, 0x29};

	unsigned char Write_29[] = {0xA2, 0x29, 0x04, 0x00, 0x00, 0x10};
	unsigned char Write_2A[] = {0xA2, 0x2A, 0x9B, 0x00, 0x00, 0x00};
	unsigned char Write_20[] = {0xA2, 0x20, 0x00, 0x00, 0x00, 0x00};

	unsigned char Write_Pass[] = {0xA2, 0x2B, 0xFF, 0xFF, 0xFF, 0xFF};

	unsigned char Read_CNT[] = {0x39, 0x02};
	unsigned char Auth[] = {0x1B, 0xFF, 0xFF, 0xFF, 0xFF};


	/* Authenticate */
	//calcula do pass

	Auth[1]=(RfIntf.Info.NFC_APP.NfcId[0]^'P')^'S';
	Auth[2]=(RfIntf.Info.NFC_APP.NfcId[1]^'I')^'O';
	Auth[3]=(RfIntf.Info.NFC_APP.NfcId[2]^'N')^'L';
	Auth[4]=(RfIntf.Info.NFC_APP.NfcId[3]^'A')^'L';

	Write_Pass[2]=(RfIntf.Info.NFC_APP.NfcId[0]^'P')^'S';
	Write_Pass[3]=(RfIntf.Info.NFC_APP.NfcId[1]^'I')^'O';
	Write_Pass[4]=(RfIntf.Info.NFC_APP.NfcId[2]^'N')^'L';
	Write_Pass[5]=(RfIntf.Info.NFC_APP.NfcId[3]^'A')^'L';

	/* Write Pass block 0x2B */

	status = NxpNci_ReaderTagCmd(Write_Pass, sizeof(Write_Pass), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_Pass[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_Pass[1]);


	/* Write config block 0x2A */

	status = NxpNci_ReaderTagCmd(Write_2A, sizeof(Write_2A), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_2A[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_2A[1]);


	/* Write config block 0x29 */

	status = NxpNci_ReaderTagCmd(Write_29, sizeof(Write_29), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_29[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_29[1]);

	/* Write config block 0x20 */

/*	Write_20[2]=0x01; //0x01 para locked=1 (libera //acesso) e 0x02 para locked=0 (funciona normal)
	status = NxpNci_ReaderTagCmd(Write_20, sizeof(Write_20), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_20[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_20[1]);
*/


	while(1);

}

void Sollus_Grava_UIDM (NxpNci_RfIntf_t RfIntf)
{
	bool status;
	unsigned char i;
	unsigned char Resp[256];
	unsigned char RespSize;
	/* Read block 4 */
	unsigned char Read[] = {0x30, 0x10};
	unsigned char Read_UIDM[] = {0x30, 0x10};
	/* Write block 4 */
	unsigned char Write[] = {0xA2, 0x2a, 0x98, 0x00, 0x00, 0x00};
	unsigned char Write_ID1[] = {0xA2, 0x10, 0x01, 0x02, 0x03, 0x04};
	unsigned char Write_ID2[] = {0xA2, 0x11, 0x05, 0x06, 0x07, 0x08};
	unsigned char Write_ID3[] = {0xA2, 0x12, 0x09, 0x0a, 0x0b, 0x0c};
	unsigned char Write_ID4[] = {0xA2, 0x13, 0x0d, 0x0e, 0x0f, 0x10};

	unsigned char Read_CNT[] = {0x39, 0x02};
	unsigned char Auth[] = {0x1B, 0xFF, 0xFF, 0xFF, 0xFF};

	unsigned int UIDM[4];
	int cont_uid;

	sim_uid_t uid;

	SIM_GetUniqueId(&uid);



	/* Authenticate */
	//calcula do pass

	Auth[1]=(RfIntf.Info.NFC_APP.NfcId[0]^'P')^'S';
	Auth[2]=(RfIntf.Info.NFC_APP.NfcId[1]^'I')^'O';
	Auth[3]=(RfIntf.Info.NFC_APP.NfcId[2]^'N')^'L';
	Auth[4]=(RfIntf.Info.NFC_APP.NfcId[3]^'A')^'L';
	/* Authenticate */
	status = NxpNci_ReaderTagCmd(Auth, sizeof(Auth), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0) || (RespSize < 3) )
	{
		printf(" Authenticate failed with error 0x%02x\n", Resp[RespSize-1]);
		acesso=2;
		return;
	}
	printf(" Authenticate succeed: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");
	acesso=1;


	/* Read_UID_MICRO */
	status = NxpNci_ReaderTagCmd(Read_UIDM, sizeof(Read_UIDM), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read_UIDM failed with error 0x%02x\n", Resp[RespSize-1]);
		return;
	}
	printf(" Read_UIDM: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");

//testar se está zerado



	//Compara se o gravado bate com o id do micro atual
	printf(" UID ARM: %08X %08X ", (int)uid.H, (int)uid.MH);
	printf("%08X %08X \n", (int)uid.ML, (int)uid.L );


	UIDM[0] = (int)uid.L;
	UIDM[1] = (int)uid.ML;
	UIDM[2] = (int)uid.MH;
	UIDM[3] = (int)uid.H;


	for (cont_uid=3;cont_uid>=0;cont_uid--) {
		Write_ID1[cont_uid+2] = UIDM[0]&0xff;
		UIDM[0] >>= 8;
	}

	for (cont_uid=3;cont_uid>=0;cont_uid--) {
		Write_ID2[cont_uid+2] = UIDM[1]&0xff;
		UIDM[1] >>= 8;
	}

	for (cont_uid=3;cont_uid>=0;cont_uid--) {
		Write_ID3[cont_uid+2] = UIDM[2]&0xff;
		UIDM[2] >>= 8;
	}
	for (cont_uid=3;cont_uid>=0;cont_uid--) {
		Write_ID4[cont_uid+2] = UIDM[3]&0xff;
		UIDM[3] >>= 8;
	}


	/* Write_ID1 */
	status = NxpNci_ReaderTagCmd(Write_ID1, sizeof(Write_ID1), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID1[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID1[1]);

	/* Write_ID2 */
	status = NxpNci_ReaderTagCmd(Write_ID2, sizeof(Write_ID2), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID2[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID2[1]);

	/* Write_ID3 */
	status = NxpNci_ReaderTagCmd(Write_ID3, sizeof(Write_ID3), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID3[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID3[1]);

	/* Write_ID4 */
	status = NxpNci_ReaderTagCmd(Write_ID4, sizeof(Write_ID4), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID4[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID4[1]);

	/* Read back */
	Read[1]=0x10;
	status = NxpNci_ReaderTagCmd(Read, sizeof(Read), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read block %d failed with error 0x%02x\n", Read[1], Resp[RespSize-1]);
		return;
	}
	printf(" Read block 0x%X: ", Read[1]); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");

}


void Sollus_Zera_UIDM (NxpNci_RfIntf_t RfIntf)
{
	bool status;
	unsigned char i;
	unsigned char Resp[256];
	unsigned char RespSize;
	/* Read block 4 */
	unsigned char Read[] = {0x30, 0x10};
	unsigned char Read_UIDM[] = {0x30, 0x10};
	/* Write block 4 */
	unsigned char Write[] = {0xA2, 0x2a, 0x98, 0x00, 0x00, 0x00};
	unsigned char Write_ID1[] = {0xA2, 0x10, 0x00, 0x00, 0x00, 0x00};
	unsigned char Write_ID2[] = {0xA2, 0x11, 0x00, 0x00, 0x00, 0x00};
	unsigned char Write_ID3[] = {0xA2, 0x12, 0x00, 0x00, 0x00, 0x00};
	unsigned char Write_ID4[] = {0xA2, 0x13, 0x00, 0x00, 0x00, 0x00};
	unsigned char Write_20[] = {0xA2, 0x20, 0x00, 0x00, 0x00, 0x00};

	unsigned char Read_CNT[] = {0x39, 0x02};
	unsigned char Auth[] = {0x1B, 0xFF, 0xFF, 0xFF, 0xFF};

	unsigned int UIDM[4];
	int cont_uid;

	sim_uid_t uid;

	SIM_GetUniqueId(&uid);



	/* Authenticate */
	//calcula do pass

	Auth[1]=(RfIntf.Info.NFC_APP.NfcId[0]^'P')^'S';
	Auth[2]=(RfIntf.Info.NFC_APP.NfcId[1]^'I')^'O';
	Auth[3]=(RfIntf.Info.NFC_APP.NfcId[2]^'N')^'L';
	Auth[4]=(RfIntf.Info.NFC_APP.NfcId[3]^'A')^'L';

	/* Authenticate */
	status = NxpNci_ReaderTagCmd(Auth, sizeof(Auth), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0) || (RespSize < 3) )
	{
		printf(" Authenticate failed with error 0x%02x\n", Resp[RespSize-1]);
		//acesso=2;
		return;
	}
	printf(" Authenticate succeed: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");
	//acesso=1;


	/* Write_ID1 */
	status = NxpNci_ReaderTagCmd(Write_ID1, sizeof(Write_ID1), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID1[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID1[1]);

	/* Write_ID2 */
	status = NxpNci_ReaderTagCmd(Write_ID2, sizeof(Write_ID2), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID2[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID2[1]);

	/* Write_ID3 */
	status = NxpNci_ReaderTagCmd(Write_ID3, sizeof(Write_ID3), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID3[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID3[1]);

	/* Write_ID4 */
	status = NxpNci_ReaderTagCmd(Write_ID4, sizeof(Write_ID4), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID4[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID4[1]);

	/* Read back */
	Read[1]=0x10;
	status = NxpNci_ReaderTagCmd(Read, sizeof(Read), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read block %d failed with error 0x%02x\n", Read[1], Resp[RespSize-1]);
		return;
	}
	printf(" Read block 0x%X: ", Read[1]); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");


		status = NxpNci_ReaderTagCmd(Write_20, sizeof(Write_20), Resp, &RespSize);
		if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
		{
			printf(" Write block %d failed with error 0x%02x\n", Write_20[1], Resp[RespSize-1]);
			return;
		}
		printf(" Block %d written\n", Write_20[1]);


	while(1);



}



void Sollus_Info (NxpNci_RfIntf_t RfIntf)
{
	bool status;
	unsigned char i,amostra;
	unsigned char Resp[256];
	unsigned char RespSize;
	/* Read block 4 */
	unsigned char Read[] = {0x30, 0x14};
	unsigned char Read_28[] = {0x30, 0x28};
	unsigned char Read_UIDM[] = {0x30, 0x10};
	unsigned char Read_locked[] = {0x30, 0x20};

	/* Write block 4 */

	unsigned char Write_Amostra[] = {0xA2, 0x14, 0x00, 0x00, 0x00, 0x00};
	unsigned char Read_CNT[] = {0x39, 0x02};
	unsigned char Auth[] = {0x1B, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned int UIDM[4], cont_uid;

	sim_uid_t uid;

	SIM_GetUniqueId(&uid);



	/* Authenticate */
	//calcula do pass

	Auth[1]=(RfIntf.Info.NFC_APP.NfcId[0]^'P')^'S';
	Auth[2]=(RfIntf.Info.NFC_APP.NfcId[1]^'I')^'O';
	Auth[3]=(RfIntf.Info.NFC_APP.NfcId[2]^'N')^'L';
	Auth[4]=(RfIntf.Info.NFC_APP.NfcId[3]^'A')^'L';

	/* Authenticate */
	status = NxpNci_ReaderTagCmd(Auth, sizeof(Auth), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0) || (RespSize < 3) )
	{
		printf(" Authenticate failed with error 0x%02x\n", Resp[0]);
		//acesso=2;
		return;
	}
	printf(" Authenticate succeed: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");

	/* Read_UID_MICRO */
	status = NxpNci_ReaderTagCmd(Read_UIDM, sizeof(Read_UIDM), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read_UIDM failed with error 0x%02x\n", Resp[RespSize-1]);
		return;
	}
	printf(" Read_UIDM: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");



	//Compara se o gravado bate com o id do micro atual
	printf(" UID ARM: %08X %08X ", (int)uid.H, (int)uid.MH);
	printf("%08X %08X \n", (int)uid.ML, (int)uid.L );


	UIDM[0] = 0;
	UIDM[1] = 0;
	UIDM[2] = 0;
	UIDM[3] = 0;


	for (cont_uid=0;cont_uid<4;cont_uid++) {
		UIDM[0] <<= 8;
		UIDM[0] |= Resp[cont_uid];
	}
	for (cont_uid=0;cont_uid<4;cont_uid++) {
		UIDM[1] <<= 8;
		UIDM[1] |= Resp[cont_uid+4];
	}
	for (cont_uid=0;cont_uid<4;cont_uid++) {
		UIDM[2] <<= 8;
		UIDM[2] |= Resp[cont_uid+8];
	}
	for (cont_uid=0;cont_uid<4;cont_uid++) {
		UIDM[3] <<= 8;
		UIDM[3] |= Resp[cont_uid+12];
	}

	printf(" UID LID: %08X %08X ", UIDM[3], UIDM[2]);
	printf("%08X %08X \n", UIDM[1], UIDM[0] );


		/* Read_CNT */
		status = NxpNci_ReaderTagCmd(Read_CNT, sizeof(Read_CNT), Resp, &RespSize);
		if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
		{
			printf(" Read_CNT failed with error 0x%02x\n", Resp[RespSize-1]);
			return;
		}
		printf(" Read_CNT: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");

		tempo=0;
		tempo <<= 8;
		tempo |= Resp[2];
		tempo <<= 8;
		tempo |= Resp[1];
		tempo <<= 8;
		tempo |= Resp[0];

		printf ("\ntempo = %d e tempo = %X\n",tempo,tempo);

	/* Read */
	status = NxpNci_ReaderTagCmd(Read, sizeof(Read), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read block %d failed with error 0x%02x\n", Read[1], Resp[RespSize-1]);
		return;
	}
	printf(" Read block 0x%X: ", Read[1]); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");

	exames=0;
	exames <<= 8;
	exames |= Resp[1];
	exames <<= 8;
	exames |= Resp[0];

	printf ("\nexames = %d e exames = %X\n",exames,exames);


	volume=100-1.531*((float)tempo/17280)-0.057*(float)exames;

	printf ("\nVolume = %d\n",(int)volume);


	/* Read back */
//	Read[1]=0x10;
	status = NxpNci_ReaderTagCmd(Read, sizeof(Read), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read block %d failed with error 0x%02x\n", Read[1], Resp[RespSize-1]);
		return;
	}
	printf(" Read block 0x%X: ", Read[1]); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");


	/* Read_28 */
	status = NxpNci_ReaderTagCmd(Read_28, sizeof(Read_28), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read_28 failed with error 0x%02x\n", Resp[RespSize-1]);
		return;
	}
	printf(" Read_28: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");


}




void PCD_ISO14443_3A_scenario (NxpNci_RfIntf_t RfIntf)
{
	bool status;
	static unsigned int teste_motor[3]={2,1}, teste_motor2[3];
	unsigned char i,amostra;
	unsigned char Resp[256];
	unsigned char RespSize;
	/* Read block 4 */
	unsigned char Read[] = {0x30, 0x14};
	unsigned char Read_28[] = {0x30, 0x28};
	unsigned char Read_UIDM[] = {0x30, 0x10};
	unsigned char Read_locked[] = {0x30, 0x20};

	/* Write block 4 */

	unsigned char Write_Amostra[] = {0xA2, 0x14, 0x00, 0x00, 0x00, 0x00};
	/*	unsigned char Write_ID1[] = {0xA2, 0x10, 0x01, 0x02, 0x03, 0x04};
	unsigned char Write_ID2[] = {0xA2, 0x11, 0x05, 0x06, 0x07, 0x08};
	unsigned char Write_ID3[] = {0xA2, 0x12, 0x09, 0x0a, 0x0b, 0x0c};
	unsigned char Write_ID4[] = {0xA2, 0x13, 0x0d, 0x0e, 0x0f, 0x10};
*/

	unsigned char Read_CNT[] = {0x39, 0x02};
	unsigned char Auth[] = {0x1B, 0xFF, 0xFF, 0xFF, 0xFF};

	unsigned int UIDM[4], cont_uid;

	sim_uid_t uid;

	SIM_GetUniqueId(&uid);



	/* Authenticate */
	//calcula do pass

	Auth[1]=(RfIntf.Info.NFC_APP.NfcId[0]^'P')^'S';
	Auth[2]=(RfIntf.Info.NFC_APP.NfcId[1]^'I')^'O';
	Auth[3]=(RfIntf.Info.NFC_APP.NfcId[2]^'N')^'L';
	Auth[4]=(RfIntf.Info.NFC_APP.NfcId[3]^'A')^'L';

	/* Authenticate */
	status = NxpNci_ReaderTagCmd(Auth, sizeof(Auth), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0) || (RespSize < 3) )
	{
		cont_erro2++;
		if( cont_erro2 >= 5 ){
			cont_erro2 = 5;
			printf(" Authenticate failed with error 0x%02x\n", Resp[0]);
			acesso=2;
		}
		return;
	}
	printf(" Authenticate succeed: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");
	acesso=1;
	cont_erro2 = 0;

	/* Read_UID_MICRO */
	status = NxpNci_ReaderTagCmd(Read_UIDM, sizeof(Read_UIDM), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read_UIDM failed with error 0x%02x\n", Resp[RespSize-1]);
		return;
	}
	printf(" Read_UIDM: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");



	//Compara se o gravado bate com o id do micro atual
	printf(" UID ARM: %08X %08X ", (int)uid.H, (int)uid.MH);
	printf("%08X %08X \n", (int)uid.ML, (int)uid.L );


	UIDM[0] = 0;
	UIDM[1] = 0;
	UIDM[2] = 0;
	UIDM[3] = 0;


	for (cont_uid=0;cont_uid<4;cont_uid++) {
		UIDM[0] <<= 8;
		UIDM[0] |= Resp[cont_uid];
	}
	for (cont_uid=0;cont_uid<4;cont_uid++) {
		UIDM[1] <<= 8;
		UIDM[1] |= Resp[cont_uid+4];
	}
	for (cont_uid=0;cont_uid<4;cont_uid++) {
		UIDM[2] <<= 8;
		UIDM[2] |= Resp[cont_uid+8];
	}
	for (cont_uid=0;cont_uid<4;cont_uid++) {
		UIDM[3] <<= 8;
		UIDM[3] |= Resp[cont_uid+12];
	}

	printf(" UID LID: %08X %08X ", UIDM[3], UIDM[2]);
	printf("%08X %08X \n", UIDM[1], UIDM[0] );

	if (!( UIDM[3] | UIDM[2] | UIDM[1] | UIDM[0] )) {
		Sollus_Grava_UIDM(RfIntf);
	}


	if ((UIDM[3] != (int)uid.H) || (UIDM[2] != (int)uid.MH) || (UIDM[1] != (int)uid.ML) || (UIDM[0] != (int)uid.L))
	{
		cont_erro3++;
		if( cont_erro3 >= 5){
			cont_erro3 = 5;
			acesso=2;
		}
		return;
	}
	else {

		cont_erro3 = 0;
		status = NxpNci_ReaderTagCmd(Read_locked, sizeof(Read_locked), Resp, &RespSize);
		if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
		{
			printf(" Read_locked failed with error 0x%02x\n", Resp[RespSize-1]);
			return;
		}
		printf(" Read_locked: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");

		if(Resp[0]==0x01) { //grava locked=1 na flash
		     /* Test pflash basic opeation only if flash is unsecure. */
		     if (kFTFx_SecurityStateNotSecure == securityStatus)
		     {
		         /* Debug message for user. */
		         /* Erase several sectors on upper pflash block where there is no code */
		         printf("\r\n Erase a sector of flash");


		 /* Erase a sector from destAdrss. */
		 #if defined(FSL_FEATURE_FLASH_HAS_PFLASH_BLOCK_SWAP) && FSL_FEATURE_FLASH_HAS_PFLASH_BLOCK_SWAP
		         /* Note: we should make sure that the sector shouldn't be swap indicator sector*/
		         destAdrss = pflashBlockBase + (pflashTotalSize - (pflashSectorSize * 2));
		 #else
		         destAdrss = pflashBlockBase + (pflashTotalSize - pflashSectorSize);
		 #endif
		         result = FLASH_Erase(&s_flashDriver, destAdrss, pflashSectorSize, kFLASH_ApiEraseKey);
		         if (kStatus_FLASH_Success != result)
		         {
		             error_trap();
		         }

		         /* Verify sector if it's been erased. */
		         result = FLASH_VerifyErase(&s_flashDriver, destAdrss, pflashSectorSize, kFTFx_MarginValueUser);
		         if (kStatus_FLASH_Success != result)
		         {
		             error_trap();
		         }

		         /* Print message for user. */
		         printf("\r\n Successfully Erased Sector 0x%x -> 0x%x\r\n", destAdrss, (destAdrss + pflashSectorSize));

		         /* Print message for user. */
		         printf("\r\n Program a buffer to a sector of flash ");
		         /* Prepare user buffer. */

/// descomentar para operação normal
		         s_buffer[0] = 0x01;
		         locked=1;

		         /// especial para carro eletrico

//		         if (locked == 0) {
//			         s_buffer[0] = 0x01;
//			         locked=1;
//		         } 	else if (locked == 1){
//			         s_buffer[0] = 0x00;
//			         locked=0;
//			         //acesso = 2;
//		         	 }


		         /* Program user buffer into flash*/
		         result = FLASH_Program(&s_flashDriver, destAdrss, s_buffer, sizeof(s_buffer));
		         if (kStatus_FLASH_Success != result)
		         {
		             error_trap();
		         }

		         /* Verify programming by Program Check command with user margin levels */
		         result = FLASH_VerifyProgram(&s_flashDriver, destAdrss, sizeof(s_buffer), s_buffer, kFTFx_MarginValueUser,
		                                      &failAddr, &failDat);
		         if (kStatus_FLASH_Success != result)
		         {
		             error_trap();
		         }
		         /* Verify programming by reading back from flash directly*/
		             s_buffer_rbc[0] = *(volatile uint32_t *)(destAdrss);
		             if (s_buffer_rbc[0] != s_buffer[0])
		             {
		                 error_trap();
		             }

		         printf("\r\n Successfully Programmed and Verified Location 0x%x -> 0x%x \r\n", destAdrss,
		                (destAdrss + sizeof(s_buffer)));

		     }
		     else
		     {
		         printf("\r\n Erase/Program opeation will not be executed, as Flash is SECURE!");
		     }

		}

		if(Resp[0]==0x02) { //grava locked=0 na flash
		     /* Test pflash basic opeation only if flash is unsecure. */
		     if (kFTFx_SecurityStateNotSecure == securityStatus)
		     {
		         /* Debug message for user. */
		         /* Erase several sectors on upper pflash block where there is no code */
		         printf("\r\n Erase a sector of flash");


		 /* Erase a sector from destAdrss. */
		 #if defined(FSL_FEATURE_FLASH_HAS_PFLASH_BLOCK_SWAP) && FSL_FEATURE_FLASH_HAS_PFLASH_BLOCK_SWAP
		         /* Note: we should make sure that the sector shouldn't be swap indicator sector*/
		         destAdrss = pflashBlockBase + (pflashTotalSize - (pflashSectorSize * 2));
		 #else
		         destAdrss = pflashBlockBase + (pflashTotalSize - pflashSectorSize);
		 #endif
		         result = FLASH_Erase(&s_flashDriver, destAdrss, pflashSectorSize, kFLASH_ApiEraseKey);
		         if (kStatus_FLASH_Success != result)
		         {
		             error_trap();
		         }

		         /* Verify sector if it's been erased. */
		         result = FLASH_VerifyErase(&s_flashDriver, destAdrss, pflashSectorSize, kFTFx_MarginValueUser);
		         if (kStatus_FLASH_Success != result)
		         {
		             error_trap();
		         }

		         /* Print message for user. */
		         printf("\r\n Successfully Erased Sector 0x%x -> 0x%x\r\n", destAdrss, (destAdrss + pflashSectorSize));

		         /* Print message for user. */
		         printf("\r\n Program a buffer to a sector of flash ");
		         /* Prepare user buffer. */
		         s_buffer[0] = 0x00;
		         locked=0;
		          /* Program user buffer into flash*/
		         result = FLASH_Program(&s_flashDriver, destAdrss, s_buffer, sizeof(s_buffer));
		         if (kStatus_FLASH_Success != result)
		         {
		             error_trap();
		         }

		         /* Verify programming by Program Check command with user margin levels */
		         result = FLASH_VerifyProgram(&s_flashDriver, destAdrss, sizeof(s_buffer), s_buffer, kFTFx_MarginValueUser,
		                                      &failAddr, &failDat);
		         if (kStatus_FLASH_Success != result)
		         {
		             error_trap();
		         }
		         /* Verify programming by reading back from flash directly*/
		             s_buffer_rbc[0] = *(volatile uint32_t *)(destAdrss);
		             if (s_buffer_rbc[0] != s_buffer[0])
		             {
		                 error_trap();
		             }

		         printf("\r\n Successfully Programmed and Verified Location 0x%x -> 0x%x \r\n", destAdrss,
		                (destAdrss + sizeof(s_buffer)));

		     }
		     else
		     {
		         printf("\r\n Erase/Program opeation will not be executed, as Flash is SECURE!");
		     }

		}


		/* Read_CNT */
		status = NxpNci_ReaderTagCmd(Read_CNT, sizeof(Read_CNT), Resp, &RespSize);
		if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
		{
			printf(" Read_CNT failed with error 0x%02x\n", Resp[RespSize-1]);
			return;
		}
		printf(" Read_CNT: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");

		tempo=0;
		tempo <<= 8;
		tempo |= Resp[2];
		tempo <<= 8;
		tempo |= Resp[1];
		tempo <<= 8;
		tempo |= Resp[0];

		printf ("\ntempo = %d e tempo = %X\n",tempo,tempo);

		//testa contador
//		if (Resp[1] >= 0x15 || Resp[2]>=0x01) //acesso=3;
	}
	/* Write */
/*	status = NxpNci_ReaderTagCmd(Write, sizeof(Write), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write[1]);
*/

	/* Read */
	status = NxpNci_ReaderTagCmd(Read, sizeof(Read), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read block %d failed with error 0x%02x\n", Read[1], Resp[RespSize-1]);
		return;
	}
	printf(" Read block 0x%X: ", Read[1]); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");

	exames=0;
	exames <<= 8;
	exames |= Resp[1];
	exames <<= 8;
	exames |= Resp[0];

	printf ("\nexames = %d e exames = %X\n",exames,exames);


	volume=100-1.531*((float)tempo/17280)-0.057*(float)exames;

	//testa volume
	if (volume <= 0) acesso=3;


	printf ("\nVolume = %d\n",(int)volume);


	/* Write_exames */
if (exame_feito==1) {
	exame_feito=0;

	exames++;
	Write_Amostra[2]=exames&0x00ff;
	exames >>= 8;
	Write_Amostra[3]=exames&0x00ff;
//	amostra=Resp[0];
//	amostra++;
//	Write_Amostra[2]=amostra;



	status = NxpNci_ReaderTagCmd(Write_Amostra, sizeof(Write_Amostra), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_Amostra[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_Amostra[1]);
}
	/* Write_ID2 */
/*	status = NxpNci_ReaderTagCmd(Write_ID2, sizeof(Write_ID2), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID2[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID2[1]);
*/
	/* Write_ID3 */
/*	status = NxpNci_ReaderTagCmd(Write_ID3, sizeof(Write_ID3), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID3[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID3[1]);
*/
	/* Write_ID4 */
/*	status = NxpNci_ReaderTagCmd(Write_ID4, sizeof(Write_ID4), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_ID4[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_ID4[1]);
*/

	/* Read back */
//	Read[1]=0x10;
	status = NxpNci_ReaderTagCmd(Read, sizeof(Read), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read block %d failed with error 0x%02x\n", Read[1], Resp[RespSize-1]);
		return;
	}
	printf(" Read block 0x%X: ", Read[1]); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");

	/* Write */
/*	status = NxpNci_ReaderTagCmd(Write, sizeof(Write), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write[1]);
*/

	/* Read_CNT */
/*	status = NxpNci_ReaderTagCmd(Read_CNT, sizeof(Read_CNT), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read_CNT failed with error 0x%02x\n", Resp[RespSize-1]);
		return;
	}
	printf(" Read_CNT: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");
*/

	/* Read_28 */
	status = NxpNci_ReaderTagCmd(Read_28, sizeof(Read_28), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read_28 failed with error 0x%02x\n", Resp[RespSize-1]);
		return;
	}
	printf(" Read_28: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");


}


void Sollus_scenario (NxpNci_RfIntf_t RfIntf)
{
	bool status;
	unsigned char i;
	unsigned char Resp[256];
	unsigned char RespSize;

	unsigned char Read_PASS[] = {0x30, 0x2b};

	unsigned char Write[] = {0xA2, 0x2a, 0x98, 0x00, 0x00, 0x00};

	unsigned char Auth[] = {0x1B, 0xFF, 0xFF, 0xFF, 0xFF};

	unsigned char Write_29[] = {0xA2, 0x29, 0x04, 0x00, 0x00, 0x10};
	unsigned char Write_2A[] = {0xA2, 0x2A, 0x98, 0x00, 0x00, 0x00};

	unsigned char Write_Pass[] = {0xA2, 0x2B, 0xFF, 0xFF, 0xFF, 0xFF};


	//Cálculo do pass

	Write_Pass[2]=RfIntf.Info.NFC_APP.NfcId[0]^'P';
	Write_Pass[3]=RfIntf.Info.NFC_APP.NfcId[1]^'I';
	Write_Pass[4]=RfIntf.Info.NFC_APP.NfcId[2]^'N';
	Write_Pass[5]=RfIntf.Info.NFC_APP.NfcId[3]^'A';

	/* Authenticate */
	status = NxpNci_ReaderTagCmd(Write_Pass, sizeof(Write_Pass), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0) || (RespSize < 3) )
	{
		printf(" Authenticate failed with error 0x%02x\n", Resp[RespSize-1]);
		//acesso=2;
		return;
	}
	printf(" Authenticate succeed: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");
	//acesso=1;

	Write_Pass[2]=(RfIntf.Info.NFC_APP.NfcId[0]^'P')^'S';
	Write_Pass[3]=(RfIntf.Info.NFC_APP.NfcId[1]^'I')^'O';
	Write_Pass[4]=(RfIntf.Info.NFC_APP.NfcId[2]^'N')^'L';
	Write_Pass[5]=(RfIntf.Info.NFC_APP.NfcId[3]^'A')^'L';

	/* Write Pass block 0x2B */

	status = NxpNci_ReaderTagCmd(Write_Pass, sizeof(Write_Pass), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_Pass[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_Pass[1]);

	/* Write config block 0x2A */

	status = NxpNci_ReaderTagCmd(Write_2A, sizeof(Write_2A), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_2A[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_2A[1]);


	/* Write config block 0x29 */

	status = NxpNci_ReaderTagCmd(Write_29, sizeof(Write_29), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write block %d failed with error 0x%02x\n", Write_29[1], Resp[RespSize-1]);
		return;
	}
	printf(" Block %d written\n", Write_29[1]);


	while(1);

	/* Read_PASS */
/*
	status = NxpNci_ReaderTagCmd(Read_PASS, sizeof(Read_PASS), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read_PASS failed with error 0x%02x\n", Resp[RespSize-1]);
		return;
	}
	printf(" Read_PASS: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");
*/


	/* Write_PASS */
/*
	status = NxpNci_ReaderTagCmd(Write_pass, sizeof(Write_pass), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Write pass failed with error 0x%02x\n", Resp[RespSize-1]);
		return;
	}
	printf(" Write pass succeed: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");
*/


	/* Read_PASS */
/*
	status = NxpNci_ReaderTagCmd(Read_PASS, sizeof(Read_PASS), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-1] != 0))
	{
		printf(" Read_PASS failed with error 0x%02x\n", Resp[RespSize-1]);
		return;
	}
	printf(" Read_PASS: "); for(i=0;i<=RespSize-2;i++) printf("0x%02X ", Resp[i]); printf("\n");
*/


}



void PCD_ISO14443_4_scenario (void)
{
	bool status;
	unsigned char Resp[256];
	unsigned char RespSize;
	unsigned char SelectPPSE[] = {0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};

	status = NxpNci_ReaderTagCmd(SelectPPSE, sizeof(SelectPPSE), Resp, &RespSize);
	if((status == NFC_ERROR) || (Resp[RespSize-2] != 0x90) || (Resp[RespSize-1] != 0x00))
	{
		printf(" Select PPSE failed with error %02x %02x\n", Resp[RespSize-2], Resp[RespSize-1]);
		return;
	}
	printf(" Select NDEF Application succeed\n");
}

#endif

void displayCardInfo(NxpNci_RfIntf_t RfIntf)
{
	switch(RfIntf.Protocol){
	case PROT_T1T:
	case PROT_T2T:
	case PROT_T3T:
	case PROT_ISODEP:
		printf(" - POLL MODE: Remote T%dT activated\n", RfIntf.Protocol);
		break;
	case PROT_ISO15693:
		printf(" - POLL MODE: Remote ISO15693 card activated\n");
		break;
	case PROT_MIFARE:
		printf(" - POLL MODE: Remote MIFARE card activated\n");
		break;
	default:
		printf(" - POLL MODE: Undetermined target\n");
		return;
	}

	switch(RfIntf.ModeTech) {
	case (MODE_POLL | TECH_PASSIVE_NFCA):
		printf("\tSENS_RES = 0x%.2x 0x%.2x\n", RfIntf.Info.NFC_APP.SensRes[0], RfIntf.Info.NFC_APP.SensRes[1]);
		print_buf("\tNFCID = ", RfIntf.Info.NFC_APP.NfcId, RfIntf.Info.NFC_APP.NfcIdLen);
		if(RfIntf.Info.NFC_APP.SelResLen != 0) printf("\tSEL_RES = 0x%.2x\n", RfIntf.Info.NFC_APP.SelRes[0]);
	break;

	case (MODE_POLL | TECH_PASSIVE_NFCB):
		if(RfIntf.Info.NFC_BPP.SensResLen != 0) print_buf("\tSENS_RES = ", RfIntf.Info.NFC_BPP.SensRes, RfIntf.Info.NFC_BPP.SensResLen);
	break;

	case (MODE_POLL | TECH_PASSIVE_NFCF):
		printf("\tBitrate = %s\n", (RfIntf.Info.NFC_FPP.BitRate==1)?"212":"424");
		if(RfIntf.Info.NFC_FPP.SensResLen != 0) print_buf("\tSENS_RES = ", RfIntf.Info.NFC_FPP.SensRes, RfIntf.Info.NFC_FPP.SensResLen);
	break;

	case (MODE_POLL | TECH_PASSIVE_15693):
		print_buf("\tID = ", RfIntf.Info.NFC_VPP.ID, sizeof(RfIntf.Info.NFC_VPP.ID));
		printf("\tAFI = 0x%.2x\n", RfIntf.Info.NFC_VPP.AFI);
		printf("\tDSFID = 0x%.2x\n", RfIntf.Info.NFC_VPP.DSFID);
	break;

	default:
		break;
	}
}

void task_nfc_reader(NxpNci_RfIntf_t RfIntf)
{
	/* For each discovered cards */
	while(1){
		/* Display detected card information */
		displayCardInfo(RfIntf);

		/* What's the detected card type ? */
		switch(RfIntf.Protocol) {
		case PROT_T1T:
		case PROT_T2T:
		case PROT_T3T:
		case PROT_ISODEP:
#ifndef RW_RAW_EXCHANGE
			/* Process NDEF message read */
			NxpNci_ProcessReaderMode(RfIntf, READ_NDEF);
#ifdef RW_NDEF_WRITING
			RW_NDEF_SetMessage ((unsigned char *) NDEF_MESSAGE, sizeof(NDEF_MESSAGE), *NdefPush_Cb);
			/* Process NDEF message write */
			NxpNci_ProcessReaderMode(RfIntf, WRITE_NDEF);
#endif
#else
			if (RfIntf.Protocol == PROT_ISODEP)
			{
				PCD_ISO14443_4_scenario();
			}
			else if (RfIntf.Protocol == PROT_T2T)
			{
				PCD_ISO14443_3A_scenario(RfIntf); //roda normal
//				Sollus_scenario(RfIntf);
//				Sollus_Grava_UIDM(RfIntf);
//				Sollus_Zera_UIDM(RfIntf);
//				Sollus_Info(RfIntf);
//				Preparacao_fabrica(RfIntf);
			}
#endif
			break;

		case PROT_ISO15693:
#ifdef RW_RAW_EXCHANGE
			/* Run dedicated scenario to demonstrate ISO15693 card management */
			ISO15693_scenario();
#endif
			break;

		case PROT_MIFARE:
#ifdef RW_RAW_EXCHANGE
			/* Run dedicated scenario to demonstrate MIFARE card management */
			MIFARE_scenario();
#endif
			break;

		default:
			break;
		}

		/* If more cards (or multi-protocol card) were discovered (only same technology are supported) select next one */
		if(RfIntf.MoreTags) {
			if(NxpNci_ReaderActivateNext(&RfIntf) == NFC_ERROR) break;
		}
		/* Otherwise leave */
		else break;
	}

	NxpNci_StopDiscovery();

	GPIO_PortClear(NXPNCI_VEN_GPIO, 1U << NXPNCI_VEN_PIN);
	Sleep( 4892 );
	GPIO_PortSet(NXPNCI_VEN_GPIO, 1U << NXPNCI_VEN_PIN);
	Sleep( 20 );
	cont_erro = 0;
	NxpNci_KeepConfiguration();
	if (NxpNci_StartDiscovery(DiscoveryTechnologies,sizeof(DiscoveryTechnologies)) != NFC_SUCCESS)
	{
		printf("Error: cannot start discovery\n");
		return;
	}
	/* Wait for card removal */
//	NxpNci_ProcessReaderMode(RfIntf, PRESENCE_CHECK);

//	printf("CARD REMOVED\n");

	/* Restart discovery loop */
	//NxpNci_StopDiscovery();
	//NxpNci_StartDiscovery(DiscoveryTechnologies,sizeof(DiscoveryTechnologies));
}
#endif

#if defined CARDEMU_SUPPORT
#ifdef CARDEMU_RAW_EXCHANGE
void PICC_ISO14443_4_scenario (void)
{
	unsigned char OK[] = {0x90, 0x00};
	unsigned char OK1[] = {0x90, 0x01};
	unsigned char OK2[] = {0x90, 0x02};

	unsigned char Cmd[256];
	unsigned char CmdSize;
	bool lock = true;

	while (1)
	{
		if(NxpNci_CardModeReceive(Cmd, &CmdSize) == NFC_SUCCESS)
		{
			if ((CmdSize >= 2) && (Cmd[0] == 0x00) && (Cmd[1] == 0xa4))
			{
				printf("Select AID received\n");
				NxpNci_CardModeSend(OK, sizeof(OK));
			}
			else if ((CmdSize >= 4) && (Cmd[0] == 0x00) && (Cmd[1] == 0xb0))
			{
	            // process read binary here
			}
			else if ((CmdSize >= 4) && (Cmd[0] == 0x00) && (Cmd[1] == 0xd0))
			{
	            // process write binary here
	            if(Cmd[3] == 0x00)
	            {
	            	lock = !lock;
	            	lock ? NxpNci_CardModeSend(OK2, sizeof(OK2)) : NxpNci_CardModeSend(OK1, sizeof(OK1));
	            }
			}
			else if ((CmdSize >= 3) && (Cmd[0] == 0x00) && (Cmd[1] == 0xfe))
			{
	            // return successful termination
				NxpNci_CardModeSend(OK, sizeof(OK));
			}
		}
		else
		{
			printf("End of transaction\n");
			return;
		}
	}
}
#endif
#endif

void task_nfc(void)
{

	NxpNci_RfIntf_t RfInterface;
	unsigned int erro_loop=0;
    /* Clean up Flash driver Structure*/
     memset(&s_flashDriver, 0, sizeof(flash_config_t));

     /* Setup flash driver structure for device and initialize variables. */
     result = FLASH_Init(&s_flashDriver);
     if (kStatus_FLASH_Success != result)
     {
         error_trap();
     }
     /* Get flash properties*/
     FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0BlockBaseAddr, &pflashBlockBase);
     FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0TotalSize, &pflashTotalSize);
     FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0SectorSize, &pflashSectorSize);

     /* print welcome message */
     printf("\r\n PFlash Example Start \r\n");
     /* Print flash information - PFlash. */
     printf("\r\n PFlash Information: ");
     printf("\r\n Total Program Flash Size:\t%d KB, Hex: (0x%x)", (pflashTotalSize / 1024), pflashTotalSize);
     printf("\r\n Program Flash Sector Size:\t%d KB, Hex: (0x%x) ", (pflashSectorSize / 1024), pflashSectorSize);

     /* Check security status. */
     result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
     if (kStatus_FLASH_Success != result)
     {
         error_trap();
     }
     /* Print security status. */
     switch (securityStatus)
     {
         case kFTFx_SecurityStateNotSecure:
             printf("\r\n Flash is UNSECURE!");
             break;
         case kFTFx_SecurityStateBackdoorEnabled:
             printf("\r\n Flash is SECURE, BACKDOOR is ENABLED!");
             break;
         case kFTFx_SecurityStateBackdoorDisabled:
             printf("\r\n Flash is SECURE, BACKDOOR is DISABLED!");
             break;
         default:
             break;
     }
     printf("\r\n");

     destAdrss = pflashBlockBase + (pflashTotalSize - (pflashSectorSize * 2));
     s_buffer_rbc[0] = *(volatile uint32_t *)(destAdrss);

     if (s_buffer_rbc[0]==0xffffffff)
    	 locked = 0;

     if (s_buffer_rbc[0]==0x01)
     	 locked = 1;


#ifdef CARDEMU_SUPPORT
	/* Register NDEF message to be sent to remote reader */
	T4T_NDEF_EMU_SetMessage((unsigned char *) NDEF_MESSAGE, sizeof(NDEF_MESSAGE), *NdefPush_Cb);
#endif

#ifdef P2P_SUPPORT
	/* Register NDEF message to be sent to remote peer */
	P2P_NDEF_SetMessage((unsigned char *) NDEF_MESSAGE, sizeof(NDEF_MESSAGE), *NdefPush_Cb);
	/* Register callback for reception of NDEF message from remote peer */
	P2P_NDEF_RegisterPullCallback(*NdefPull_Cb);
#endif

#ifdef RW_SUPPORT
	/* Register callback for reception of NDEF message from remote cards */
	RW_NDEF_RegisterPullCallback(*NdefPull_Cb);
#endif

	/* Open connection to NXPNCI device */
#ifdef app_run	// Função sem configuração de dados para PN7150

	tml_Connect ();
	if( NxpNci_KeepConfiguration() != NFC_SUCCESS ){

		erro_loop = 1;
	}

	if (erro_loop!=0) {
		printf("Error: problema de comunicacao com a placa nfc\n");
		////acesso=4;						//sem comunicação com o nfc
		while(1) vTaskDelay(1000);
		//seta led para indicação de erro
	}
#endif

#ifdef fabrica
	erro_loop=0;
		if (NxpNci_Connect() == NFC_ERROR) {
			printf("Error: cannot connect to NXPNCI device\n");
//			return;
			erro_loop=1;
		}

		if (NxpNci_ConfigureSettings() == NFC_ERROR) {
			printf("Error: cannot configure NXPNCI settings\n");
//			return;
			erro_loop=1;
		}


		if (NxpNci_ConfigureMode(mode) == NFC_ERROR)
		{
			printf("Error: cannot configure NXPNCI\n");
//			return;
			erro_loop=1;
		}

if (erro_loop!=0) {
	printf("Error: problema de comunicacao com a placa nfc\n");
	//acesso=4;						//sem comunicação com o nfc
	while(1) vTaskDelay(1000);
	//seta led para indicação de erro
}
#endif
	/* Start Discovery */

		if (NxpNci_StartDiscovery(DiscoveryTechnologies,sizeof(DiscoveryTechnologies)) != NFC_SUCCESS)
		{
			printf("Error: cannot start discovery\n");
//			return;
			erro_loop=1;
		}

	while(1)
	{
		printf("\nWAITING FOR DEVICE DISCOVERY\n");
		/* Wait until a peer is discovered */
		if (NxpNci_WaitForDiscoveryNotification(&RfInterface))
			{

			cont_erro++;
			if( cont_erro > 5 ){
				cont_erro = 5;
//				if(locked==0)
//					//acesso=0; //só altera //acesso para zero se der time out
//				if(locked==1)
//					//acesso=1; //se o sistema estiver desabilitado, libera //acesso
			}

			}
		else {

#ifdef CARDEMU_SUPPORT
		/* Is activated from remote T4T ? */
		if ((RfInterface.Interface == INTF_ISODEP) && ((RfInterface.ModeTech & MODE_MASK) == MODE_LISTEN))
		{
			printf(" - LISTEN MODE: Activated from remote Reader\n");
#ifndef CARDEMU_RAW_EXCHANGE
			NxpNci_ProcessCardMode(RfInterface);
#else
			PICC_ISO14443_4_scenario();
#endif
			printf("READER DISCONNECTED\n");
		}
		else
#endif

#ifdef P2P_SUPPORT
		/* Is activated from remote T4T ? */
		if (RfInterface.Interface == INTF_NFCDEP)
		{
			if ((RfInterface.ModeTech & MODE_LISTEN) == MODE_LISTEN) printf(" - P2P TARGET MODE: Activated from remote Initiator\n");
			else printf(" - P2P INITIATOR MODE: Remote Target activated\n");

			/* Process with SNEP for NDEF exchange */
			NxpNci_ProcessP2pMode(RfInterface);

			printf("PEER LOST\n");
		}
		else
#endif
#ifdef RW_SUPPORT
		if ((RfInterface.ModeTech & MODE_MASK) == MODE_POLL)
		{
			task_nfc_reader(RfInterface);
		}
		else
#endif
		{
			printf("WRONG DISCOVERY\n");
		}
		}
	}
}
