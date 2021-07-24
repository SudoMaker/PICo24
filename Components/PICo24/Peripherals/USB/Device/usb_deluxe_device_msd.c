/*
    This file is part of PotatoPi PICo24 SDK.

    Copyright (C) 2021 ReimuNotMoe <reimu@sudomaker.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.


    This file incorporates work covered by the following copyright and
    permission notice:

    Copyright 2015 Microchip Technology Inc. (www.microchip.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "usb_deluxe_device.h"
#include "usb_deluxe_device_msd.h"
#include <PICo24/Library/SafeMalloc.h>


extern volatile CTRL_TRF_SETUP SetupPkt;

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MSD

static const USBDeluxeDevice_MSD_InquiryResponse msd_inq_resp = {
	0x00,		// peripheral device is connected, direct access block device
	0x80,           // removable
	0x04,	 	// version = 00=> does not conform to any standard, 4=> SPC-2
	0x02,		// response is in format specified by SPC-2
	0x20,		// n-4 = 36-4=32= 0x20
	0x00,		// sccs etc.
	0x00,		// bque=1 and cmdque=0,indicates simple queueing 00 is obsolete,
	// but as in case of other device, we are just using 00
	0x00,		// 00 obsolete, 0x80 for basic task queueing
	{'P', 'I', 'C', 'o', '2', '4'},
	// this is the T10 assigned Vendor ID
	{'M','a','s','s',' ','S','t','o','r','a','g','e',' ',' ',' ',' '},
	{'0','0','0','1'}
};

extern volatile uint8_t CtrlTrfData[USB_EP0_BUFF_SIZE];

void USBDeluxeDevice_MSD_ResetSenseData(USBDeluxeDevice_MSDContext *msd_ctx, uint8_t LUN_INDEX);
uint8_t USBDeluxeDevice_MSD_ProcessCommand(USBDeluxeDevice_MSDContext *msd_ctx);
void USBDeluxeDevice_MSD_ProcessCommandMediaPresent(USBDeluxeDevice_MSDContext *msd_ctx);
void USBDeluxeDevice_MSD_ProcessCommandMediaAbsent(USBDeluxeDevice_MSDContext *msd_ctx);

void MSDComputeDeviceInAndResidue(USBDeluxeDevice_MSDContext *msd_ctx, uint16_t DiExpected);
uint8_t MSDCheckForErrorCases(USBDeluxeDevice_MSDContext *msd_ctx, uint32_t DeviceBytes);
void MSDErrorHandler(USBDeluxeDevice_MSDContext *msd_ctx, uint8_t ErrorCase);

uint8_t MSDReadHandler(USBDeluxeDevice_MSDContext *msd_ctx);
uint8_t MSDWriteHandler(USBDeluxeDevice_MSDContext *msd_ctx);

void USBDeluxeDevice_MSD_Create(USBDeluxeDevice_MSDContext *msd_ctx, void *userp, uint8_t usb_iface, uint8_t usb_ep_in, uint8_t usb_ep_out,
				uint8_t nr_luns, const char *vendor_id, const char *product_id, const char *product_rev, USBDeluxeDevice_MSD_DiskOps *diskops) {
	memset(msd_ctx, 0, sizeof(USBDeluxeDevice_MSDContext));
	msd_ctx->userp = userp;
	msd_ctx->USB_IFACE = usb_iface;
	msd_ctx->USB_EP_IN = usb_ep_in;
	msd_ctx->USB_EP_OUT = usb_ep_out;
	msd_ctx->nr_LUNs = nr_luns;
	msd_ctx->gblSenseData = calloc_safe(nr_luns, sizeof(USBDeluxeDevice_MSD_RequestSenseResponse));
	msd_ctx->vendor_id = vendor_id;
	msd_ctx->product_id = product_id;
	msd_ctx->product_rev = product_rev;
	memcpy(&msd_ctx->diskops, diskops, sizeof(USBDeluxeDevice_MSD_DiskOps));
}

void USBDeluxeDevice_MSD_Init(USBDeluxeDevice_MSDContext *msd_ctx) {
	if (msd_ctx->USB_EP_IN == msd_ctx->USB_EP_OUT) {
		USBEnableEndpoint(msd_ctx->USB_EP_IN, USB_IN_ENABLED | USB_OUT_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);
	} else {
		USBEnableEndpoint(msd_ctx->USB_EP_IN, USB_IN_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);
		USBEnableEndpoint(msd_ctx->USB_EP_OUT, USB_OUT_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);
	}

	//Prepare to receive the first CBW
	msd_ctx->USBMSDOutHandle = USBRxOnePacket(msd_ctx->USB_EP_OUT,(uint8_t*)&msd_ctx->msd_cbw,MSD_OUT_EP_SIZE);
	//Initialize IN handle to point to first available IN MSD bulk endpoint entry
	msd_ctx->USBMSDInHandle = USBGetNextHandle(msd_ctx->USB_EP_IN, IN_TO_HOST);
	msd_ctx->MSD_State = MSD_WAIT;
	msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
	msd_ctx->MSDReadState = MSD_READ10_WAIT;
	msd_ctx->MSDWriteState = MSD_WRITE10_WAIT;
	msd_ctx->MSDHostNoData = false;
	msd_ctx->gblNumBLKS.Val = 0;
	msd_ctx->gblBLKLen.Val = 0;
	msd_ctx->MSDCBWValid = true;

	msd_ctx->gblMediaPresent = 0;

	//For each of the possible logical units
	for (uint8_t cur_lun=0; cur_lun<msd_ctx->nr_LUNs; cur_lun++) {
		//clear all of the soft detach variables

//		msd_ctx->SoftDetach[cur_lun] = false;

		//see if the media is attached
		if (msd_ctx->diskops.MediaDetect(msd_ctx->userp, cur_lun)) {
			//initialize the media
			if(msd_ctx->diskops.MediaInitialize(msd_ctx->userp, cur_lun)) {
				//if the media was present and successfully initialized
				//  then mark and indicator that the media is ready
				msd_ctx->gblMediaPresent |= ((uint16_t)1 << (cur_lun));
			}
		}

		USBDeluxeDevice_MSD_ResetSenseData(msd_ctx, cur_lun);
	}
}

void USBDeluxeDevice_MSD_ResetSenseData(USBDeluxeDevice_MSDContext *msd_ctx, uint8_t LUN_INDEX) {
	msd_ctx->gblSenseData[LUN_INDEX].ResponseCode=S_CURRENT;
	msd_ctx->gblSenseData[LUN_INDEX].VALID=0;			// no data in the information field
	msd_ctx->gblSenseData[LUN_INDEX].Obsolete=0x0;
	msd_ctx->gblSenseData[LUN_INDEX].SenseKey=S_NO_SENSE;
	//gblSenseData.Resv;
	msd_ctx->gblSenseData[LUN_INDEX].ILI=0;
	msd_ctx->gblSenseData[LUN_INDEX].EOM=0;
	msd_ctx->gblSenseData[LUN_INDEX].FILEMARK=0;
	msd_ctx->gblSenseData[LUN_INDEX].InformationB0=0x00;
	msd_ctx->gblSenseData[LUN_INDEX].InformationB1=0x00;
	msd_ctx->gblSenseData[LUN_INDEX].InformationB2=0x00;
	msd_ctx->gblSenseData[LUN_INDEX].InformationB3=0x00;
	msd_ctx->gblSenseData[LUN_INDEX].AddSenseLen=0x0a;	// n-7 (n=17 (0..17))
	msd_ctx->gblSenseData[LUN_INDEX].CmdSpecificInfo.Val=0x0;
	msd_ctx->gblSenseData[LUN_INDEX].ASC=0x0;
	msd_ctx->gblSenseData[LUN_INDEX].ASCQ=0x0;
	msd_ctx->gblSenseData[LUN_INDEX].FRUC=0x0;
	msd_ctx->gblSenseData[LUN_INDEX].SenseKeySpecific[0]=0x0;
	msd_ctx->gblSenseData[LUN_INDEX].SenseKeySpecific[1]=0x0;
	msd_ctx->gblSenseData[LUN_INDEX].SenseKeySpecific[2]=0x0;
}

uint8_t USBDeluxeDevice_MSD_CurrentLUN(USBDeluxeDevice_MSDContext *msd_ctx) {
	return msd_ctx->gblCBW.bCBWLUN;
}

void USBDeluxeDevice_MSD_CheckRequest(USBDeluxeDevice_MSDContext *msd_ctx) {
	if (SetupPkt.Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD) {
		return;
	}

	if (SetupPkt.bIntfID != msd_ctx->USB_IFACE) {
		return;
	}

	switch (SetupPkt.bRequest) {
		case MSD_RESET:
			//First make sure all request parameters are correct:
			//MSD BOT specs require wValue to be == 0x0000 and wLength == 0x0000
			if((SetupPkt.wValue != 0) || (SetupPkt.wLength != 0)) {
				return; //Return without handling the request (results in STALL)
			}

			//Host would typically issue this after a STALL event on an MSD
			//bulk endpoint.  The MSD reset should re-initialize status
			//so as to prepare for a new CBW.  Any currently ongoing command
			//block should be aborted, but the STALL and DTS states need to be
			//maintained (host will re-initialize these separately using
			//CLEAR_FEATURE, endpoint halt).
			msd_ctx->MSD_State = MSD_WAIT;
			msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
			msd_ctx->MSDReadState = MSD_READ10_WAIT;
			msd_ctx->MSDWriteState = MSD_WRITE10_WAIT;
			msd_ctx->MSDCBWValid = true;
			//Need to re-arm MSD bulk OUT endpoint, if it isn't currently armed,
			//to be able to receive next CBW.  If it is already armed, don't need
			//to do anything, since we can already receive the next CBW (or we are
			//STALLed, and the host will issue clear halt first).
			if (!USBHandleBusy(USBGetNextHandle(msd_ctx->USB_EP_OUT, OUT_FROM_HOST))) {
				msd_ctx->USBMSDOutHandle = USBRxOnePacket(msd_ctx->USB_EP_OUT,(uint8_t*)&msd_ctx->msd_cbw, MSD_OUT_EP_SIZE);
			}

			//Let USB stack know we took care of handling the EP0 SETUP request.
			//Allow zero byte status stage to proceed normally now.
			USBEP0Transmit(USB_EP0_NO_DATA);
			break;

		case GET_MAX_LUN:
			//First make sure all request parameters are correct:
			//MSD BOT specs require wValue to be == 0x0000, and wLengh == 1
			if((SetupPkt.wValue != 0) || (SetupPkt.wLength != 1)) {
				break;  //Return without handling the request (results in STALL)
			}

			//If the host asks for the maximum number of logical units
			//  then send out a packet with that information
			CtrlTrfData[0] = msd_ctx->nr_LUNs - 1;
			USBEP0SendRAMPtr((uint8_t*)&CtrlTrfData[0], 1, USB_EP0_INCLUDE_ZERO);
			break;
	}	//end switch(SetupPkt.bRequest)
}

uint8_t USBDeluxeDevice_MSD_Tasks(USBDeluxeDevice_MSDContext *msd_ctx) {
	uint8_t i;

	//Error check to make sure we have are in the CONFIGURED_STATE, prior to
	//performing MSDTasks().  Some of the MSDTasks require that the device be
	//configured first.
//	if (USBGetDeviceState() != CONFIGURED_STATE) {
//		return MSD_WAIT;
//	}

	//Note: Both the USB stack code (usb_device.c) and this MSD handler code
	//have the ability to modify the BDT values for the MSD bulk endpoints.  If the
	//USB stack operates in USB_INTERRUPT mode (user option in usb_config.h), we
	//should temporarily disable USB interrupts, to avoid any possibility of both
	//the USB stack and this MSD handler from modifying the same BDT entry, or
	//MSD state machine variables (ex: in the case of MSD_RESET) at the same time.
	USBMaskInterrupts();

	//Main MSD task dispatcher.  Receives MSD Command Block Wrappers (CBW) and
	//dispatches appropriate lower level handlers to service the requests.
	switch (msd_ctx->MSD_State) {
		case MSD_WAIT: //idle state, when we are waiting for a command from the host
		{
			//Check if we have received a new command block wrapper (CBW)
			if(!USBHandleBusy(msd_ctx->USBMSDOutHandle)) {
				//If we are in the MSD_WAIT state, and we received an OUT transaction
				//on the MSD OUT endpoint, then we must have just received an MSD
				//Command Block Wrapper (CBW).
				//First copy the the received data to to the gblCBW structure, so
				//that we keep track of the command, but free up the MSD OUT endpoint
				//buffer for fulfilling whatever request may have been received.
				//gblCBW = msd_cbw; //we are doing this, but below method can yield smaller code size
				for(i = 0; i < MSD_CBW_SIZE; i++) {
					*((uint8_t*)&msd_ctx->gblCBW.dCBWSignature + i) = *((uint8_t*)&msd_ctx->msd_cbw.dCBWSignature + i);
				}

				//If this CBW is valid?
				if((USBHandleGetLength(msd_ctx->USBMSDOutHandle) == MSD_CBW_SIZE) && (msd_ctx->gblCBW.dCBWSignature == MSD_VALID_CBW_SIGNATURE)) {
					//The CBW was valid, set flag meaning any stalls after this point
					//should not be "persistent" (as in the case of non-valid CBWs).
					msd_ctx->MSDCBWValid = true;

					//Is this CBW meaningful?
					if((msd_ctx->gblCBW.bCBWLUN <= msd_ctx->nr_LUNs - 1)                                      //Verify the command is addressed to a supported LUN
					   &&(msd_ctx->gblCBW.bCBWCBLength <= MSD_MAX_CB_SIZE)                          //Verify the claimed CB length is reasonable/valid
					   &&(msd_ctx->gblCBW.bCBWCBLength >= 0x01)                                     //Verify the claimed CB length is reasonable/valid
					   &&((msd_ctx->gblCBW.bCBWFlags & MSD_CBWFLAGS_RESERVED_BITS_MASK) == 0x00))   //Verify reserved bits are clear
					{

						//The CBW was both valid and meaningful.
						//Begin preparing a valid Command Status Wrapper (CSW),
						//in anticipation of completing the request successfully.
						//If an error detected is later, we will change the status
						//before sending the CSW.
						msd_ctx->msd_csw.dCSWSignature = MSD_VALID_CSW_SIGNATURE;
						msd_ctx->msd_csw.dCSWTag = msd_ctx->gblCBW.dCBWTag;
						msd_ctx->msd_csw.dCSWDataResidue = 0x0;
						msd_ctx->msd_csw.bCSWStatus = MSD_CSW_COMMAND_PASSED;

						//Since a new CBW just arrived, we should re-init the
						//lower level state machines to their default states.
						//Even if the prior operation didn't fully complete
						//normally, we should abandon the prior operation, when
						//a new CBW arrives.
						msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
						msd_ctx->MSDReadState = MSD_READ10_WAIT;
						msd_ctx->MSDWriteState = MSD_WRITE10_WAIT;

						//Keep track of retry attempts, in case of temporary
						//failures during read or write of the media.
						msd_ctx->MSDRetryAttempt = 0;

						//Check the command.  With the exception of the REQUEST_SENSE
						//command, we should reset the sense key info for each new command block.
						//Assume the command will get processed successfully (and hence "NO SENSE"
						//response, which is used for success cases), unless handler code
						//later on detects some kind of error.  If it does, it should
						//update the sense keys to reflect the type of error detected,
						//prior to sending the CSW.
						if(msd_ctx->gblCBW.CBWCB[0] != MSD_REQUEST_SENSE) {
							msd_ctx->gblSenseData[msd_ctx->gblCBW.bCBWLUN].SenseKey = S_NO_SENSE;
							msd_ctx->gblSenseData[msd_ctx->gblCBW.bCBWLUN].ASC = ASC_NO_ADDITIONAL_SENSE_INFORMATION;
							msd_ctx->gblSenseData[msd_ctx->gblCBW.bCBWLUN].ASCQ = ASCQ_NO_ADDITIONAL_SENSE_INFORMATION;
						}

						//Isolate the data direction bit.  The direction bit is bit 7 of the bCBWFlags byte.
						//Then, based on the direction of the data transfer, prepare the MSD state machine
						//so it knows how to proceed with processing the request.
						//If bit7 = 0, then direction is OUT from host.  If bit7 = 1, direction is IN to host
						if (msd_ctx->gblCBW.bCBWFlags & MSD_CBW_DIRECTION_BITMASK) {
							msd_ctx->MSD_State = MSD_DATA_IN;
						}
						else //else direction must be OUT from host
						{
							msd_ctx->MSD_State = MSD_DATA_OUT;
						}

						//Determine if the host is expecting there to be data transfer or not.
						//Doing this now will make for quicker error checking later.
						if (msd_ctx->gblCBW.dCBWDataTransferLength != 0) {
							msd_ctx->MSDHostNoData = false;
						} else {
							msd_ctx->MSDHostNoData = true;
						}

						//Copy the received command to the lower level command
						//state machine, so it knows what to do.
						msd_ctx->MSDCommandState = msd_ctx->gblCBW.CBWCB[0];
					} else {
						//else the CBW wasn't meaningful.  Section 6.4 of BOT specs v1.0 says,
						//"The response of a device to a CBW that is not meaningful is not specified."
						//Lets STALL the bulk endpoints, so as to promote the possibility of recovery.
						USBStallEndpoint(msd_ctx->USB_EP_IN, IN_TO_HOST);
						USBStallEndpoint(msd_ctx->USB_EP_OUT, OUT_FROM_HOST);
					}
				}//end of: if((USBHandleGetLength(USBMSDOutHandle) == MSD_CBW_SIZE) && (gblCBW.dCBWSignature == MSD_VALID_CBW_SIGNATURE))
				else  //The CBW was not valid.
				{
					//Section 6.6.1 of the BOT specifications rev. 1.0 says the device shall STALL bulk IN and OUT
					//endpoints (or should discard OUT data if not stalled), and should stay in this state until a
					//"Reset Recovery" (MSD Reset + clear endpoint halt commands on EP0, see section 5.3.4)
					USBStallEndpoint(msd_ctx->USB_EP_IN, IN_TO_HOST);
					USBStallEndpoint(msd_ctx->USB_EP_OUT, OUT_FROM_HOST);
					msd_ctx->MSDCBWValid = false;    //Flag so as to enable a "persistent"
					//stall (cannot be cleared by clear endpoint halt, unless preceded
					//by an MSD reset).
				}
			}//if(!USBHandleBusy(USBMSDOutHandle))
			break;
		}//end of: case MSD_WAIT:
		case MSD_DATA_IN:
			if (USBDeluxeDevice_MSD_ProcessCommand(msd_ctx) == MSD_COMMAND_WAIT) {
				// Done processing the command, send the status
				msd_ctx->MSD_State = MSD_SEND_CSW;
			}
			break;
		case MSD_DATA_OUT:
			if (USBDeluxeDevice_MSD_ProcessCommand(msd_ctx) == MSD_COMMAND_WAIT) {
				/* Finished receiving the data prepare and send the status */
				if ((msd_ctx->msd_csw.bCSWStatus == MSD_CSW_COMMAND_PASSED) && (msd_ctx->msd_csw.dCSWDataResidue!=0)) {
					msd_ctx->msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
				}
				msd_ctx->MSD_State = MSD_SEND_CSW;
			}
			break;
		case MSD_SEND_CSW:
			//Check to make sure the bulk IN endpoint is available before sending CSW.
			//The endpoint might still be busy sending the last packet on the IN endpoint.
			if (USBHandleBusy(USBGetNextHandle(msd_ctx->USB_EP_IN, IN_TO_HOST)) == true) {
				break;  //Not available yet.  Just stay in this state and try again later.
			}

			//Send the Command Status Wrapper (CSW) packet
			msd_ctx->USBMSDInHandle = USBTxOnePacket(msd_ctx->USB_EP_IN,(uint8_t*)&msd_ctx->msd_csw, MSD_CSW_SIZE);
			//If the bulk OUT endpoint isn't already armed, make sure to do so
			//now so we can receive the next CBW packet from the host.
			if(!USBHandleBusy(msd_ctx->USBMSDOutHandle)) {
				msd_ctx->USBMSDOutHandle = USBRxOnePacket(msd_ctx->USB_EP_OUT,(uint8_t*)&msd_ctx->msd_cbw, MSD_OUT_EP_SIZE);
			}
			msd_ctx->MSD_State = MSD_WAIT;
			break;
		default:
			//Illegal condition that should not happen, but might occur if the
			//device firmware incorrectly calls MSDTasks() prior to calling
			//USBMSDInit() during the set-configuration portion of enumeration.
			msd_ctx->MSD_State = MSD_WAIT;
			msd_ctx->msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
			USBStallEndpoint(msd_ctx->USB_EP_OUT, OUT_FROM_HOST);
	}//switch(MSD_State)

	//Safe to re-enable USB interrupts now.
	USBUnmaskInterrupts();

	return msd_ctx->MSD_State;
}

void MSDComputeDeviceInAndResidue(USBDeluxeDevice_MSDContext *msd_ctx, uint16_t DiExpected) {
	//Error check number of bytes to send.  Check for Hi < Di
	if (msd_ctx->gblCBW.dCBWDataTransferLength < DiExpected) {
		//The host has requested less data than the entire response.  We
		//send only the host requested quantity of bytes.
		msd_ctx->msd_csw.dCSWDataResidue = 0;
		msd_ctx->TransferLength.Val = msd_ctx->gblCBW.dCBWDataTransferLength;
	} else {
		//The host requested greater than or equal to the number of bytes expected.
		if (DiExpected < msd_ctx->TransferLength.Val) {
			msd_ctx->TransferLength.Val = DiExpected;
		}
		msd_ctx->msd_csw.dCSWDataResidue = msd_ctx->gblCBW.dCBWDataTransferLength - msd_ctx->TransferLength.Val;
	}
}

void MSDErrorHandler(USBDeluxeDevice_MSDContext *msd_ctx, uint8_t ErrorCase) {
	uint8_t OldMSD_State;

	//Both MSD bulk IN and OUT endpoints should not be busy when these error cases are detected
	//If for some reason this isn't true, then we should preserve the state machines states for now.
	if ((USBHandleBusy(msd_ctx->USBMSDInHandle)) || (USBHandleBusy(msd_ctx->USBMSDOutHandle))) {
		return;
	}

	//Save the old state before we change it.  The old state is needed to determine
	//the proper handling behavior in the case of receiving unsupported commands.
	OldMSD_State = msd_ctx->MSD_State;

	//Reset main state machines back to idle values.
	msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
	msd_ctx->MSDReadState = MSD_READ10_WAIT;
	msd_ctx->MSDWriteState = MSD_WRITE10_WAIT;
	//After the conventional 13 test cases failures, the host still expects a valid CSW packet
	msd_ctx->msd_csw.dCSWDataResidue = msd_ctx->gblCBW.dCBWDataTransferLength; //Indicate the un-consumed/unsent data
	msd_ctx->msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;    //Gets changed later to phase error for errors that user phase error
	msd_ctx->MSD_State = MSD_SEND_CSW;

	//Now do other error related handling tasks, which depend on the specific
	//error	type that was detected.
	switch (ErrorCase) {
		case MSD_ERROR_CASE_2://Also CASE_3
			msd_ctx->msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
			break;

		case MSD_ERROR_CASE_4://Also CASE_5
			USBStallEndpoint(msd_ctx->USB_EP_IN, IN_TO_HOST);	//STALL the bulk IN MSD endpoint
			break;

		case MSD_ERROR_CASE_7://Also CASE_8
			msd_ctx->msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
			USBStallEndpoint(msd_ctx->USB_EP_IN, IN_TO_HOST);	//STALL the bulk IN MSD endpoint
			break;

		case MSD_ERROR_CASE_9://Also CASE_11
			USBStallEndpoint(msd_ctx->USB_EP_OUT, OUT_FROM_HOST); //Stall the bulk OUT endpoint
			break;

		case MSD_ERROR_CASE_10://Also CASE_13
			msd_ctx->msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
			USBStallEndpoint(msd_ctx->USB_EP_OUT, OUT_FROM_HOST);
			break;

		case MSD_ERROR_UNSUPPORTED_COMMAND:
			USBDeluxeDevice_MSD_ResetSenseData(msd_ctx, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx));
			msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].SenseKey=S_ILLEGAL_REQUEST;
			msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASC=ASC_INVALID_COMMAND_OPCODE;
			msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASCQ=ASCQ_INVALID_COMMAND_OPCODE;

			if ((OldMSD_State == MSD_DATA_OUT) && (msd_ctx->gblCBW.dCBWDataTransferLength != 0)) {
				USBStallEndpoint(msd_ctx->USB_EP_OUT, OUT_FROM_HOST);
			} else {
				USBStallEndpoint(msd_ctx->USB_EP_IN, IN_TO_HOST);
			}
			break;
		default:	//Shouldn't get hit, don't call MSDErrorHandler() if there is no error
			break;
	}//switch(ErrorCase)
}


uint8_t MSDCheckForErrorCases(USBDeluxeDevice_MSDContext *msd_ctx, uint32_t DeviceBytes) {
	uint8_t MSDErrorCase;
	bool HostMoreDataThanDevice;
	bool DeviceNoData;

	//Check if device is expecting no data (Dn)
	if (DeviceBytes == 0) {
		DeviceNoData = true;
	} else {
		DeviceNoData = false;
	}

	//First check for the three good/non-error cases

	//Check for good case: Hn = Dn (Case 1)
	if ((msd_ctx->MSDHostNoData == true) && (DeviceNoData == true)) {
		return MSD_ERROR_CASE_NO_ERROR;
	}

	//Check for good cases where the data sizes between host and device match
	if (msd_ctx->gblCBW.dCBWDataTransferLength == DeviceBytes) {
		//Check for good case: Hi = Di (Case 6)
		if (msd_ctx->MSD_State == MSD_DATA_IN) {
			//Make sure Hi = Di, instead of Hi = Do
			if (msd_ctx->MSDCommandState != MSD_WRITE_10) {
				return MSD_ERROR_CASE_NO_ERROR;
			}
		} else //if(MSD_State == MSD_DATA_OUT)
		{
			//Check for good case: Ho = Do (Case 12)
			//Make sure Ho = Do, instead of Ho = Di
			if (msd_ctx->MSDCommandState == MSD_WRITE_10) {
				return MSD_ERROR_CASE_NO_ERROR;
			}
		}
	}

	//If we get to here, this implies some kind of error is occurring.  Do some
	//checks to find out which error occurred, so we know how to handle it.

	//Check if the host is expecting to transfer more bytes than the device. (Hx > Dx)
	if (msd_ctx->gblCBW.dCBWDataTransferLength > DeviceBytes) {
		HostMoreDataThanDevice = true;
	} else {
		HostMoreDataThanDevice = false;
	}

	//Check host's expected data direction
	if (msd_ctx->MSD_State == MSD_DATA_OUT) {
		//First check for Ho <> Di (Case 10)
		if ((msd_ctx->MSDCommandState != MSD_WRITE_10) && (DeviceNoData == false))
			MSDErrorCase = MSD_ERROR_CASE_10;
			//Check for Hn < Do  (Case 3)
		else if (msd_ctx->MSDHostNoData == true)
			MSDErrorCase = MSD_ERROR_CASE_3;
			//Check for Ho > Dn  (Case 9)
		else if (DeviceNoData == true)
			MSDErrorCase = MSD_ERROR_CASE_9;
			//Check for Ho > Do  (Case 11)
		else if(HostMoreDataThanDevice == true)
			MSDErrorCase = MSD_ERROR_CASE_11;
			//Check for Ho < Do  (Case 13)
		else //if(gblCBW.dCBWDataTransferLength < DeviceBytes)
			MSDErrorCase = MSD_ERROR_CASE_13;
	} else //else the MSD_State must be == MSD_DATA_IN
	{
		//First check for Hi <> Do (Case 8)
		if (msd_ctx->MSDCommandState == MSD_WRITE_10)
			MSDErrorCase = MSD_ERROR_CASE_8;
			//Check for Hn < Di  (Case 2)
		else if (msd_ctx->MSDHostNoData == true)
			MSDErrorCase = MSD_ERROR_CASE_2;
			//Check for Hi > Dn  (Case 4)
		else if (DeviceNoData == true)
			MSDErrorCase = MSD_ERROR_CASE_4;
			//Check for Hi > Di  (Case 5)
		else if (HostMoreDataThanDevice == true)
			MSDErrorCase = MSD_ERROR_CASE_5;
			//Check for Hi < Di  (Case 7)
		else //if(gblCBW.dCBWDataTransferLength < DeviceBytes)
			MSDErrorCase = MSD_ERROR_CASE_7;
	}
	//Now call the MSDErrorHandler(), based on the error that was detected.
	MSDErrorHandler(msd_ctx, MSDErrorCase);
	return MSDErrorCase;
}

uint8_t MSDReadHandler(USBDeluxeDevice_MSDContext *msd_ctx) {
	switch (msd_ctx->MSDReadState) {
		case MSD_READ10_WAIT:
			//Extract the LBA from the CBW.  Note: Also need to perform endian
			//swap, since the multi-byte CBW fields are stored big endian, but
			//the Microchip C compilers are little endian.
			msd_ctx->LBA.v[3]=msd_ctx->gblCBW.CBWCB[2];
			msd_ctx->LBA.v[2]=msd_ctx->gblCBW.CBWCB[3];
			msd_ctx->LBA.v[1]=msd_ctx->gblCBW.CBWCB[4];
			msd_ctx->LBA.v[0]=msd_ctx->gblCBW.CBWCB[5];

			msd_ctx->TransferLength.byte.HB = msd_ctx->gblCBW.CBWCB[7];   //MSB of Transfer Length (in number of blocks, not bytes)
			msd_ctx->TransferLength.byte.LB = msd_ctx->gblCBW.CBWCB[8];   //LSB of Transfer Length (in number of blocks, not bytes)

			//Check for possible error cases before proceeding
			if (MSDCheckForErrorCases(msd_ctx, msd_ctx->TransferLength.Val * (uint32_t)FILEIO_CONFIG_MEDIA_SECTOR_SIZE) != MSD_ERROR_CASE_NO_ERROR) {
				break;
			}

			msd_ctx->MSDReadState = MSD_READ10_BLOCK;
			//Fall through to MSD_READ_BLOCK
		case MSD_READ10_BLOCK:
			if (msd_ctx->TransferLength.Val == 0) {
				msd_ctx->MSDReadState = MSD_READ10_WAIT;
				break;
			}

			msd_ctx->TransferLength.Val--;					// we have read 1 LBA
			msd_ctx->MSDReadState = MSD_READ10_SECTOR;
			//Fall through to MSD_READ10_SECTOR

		case MSD_READ10_SECTOR:
			//if the old data isn't completely sent yet
			if (USBHandleBusy(msd_ctx->USBMSDInHandle) != 0) {
				break;
			}

			//Try to read a sector worth of data from the media, but check for
			//possible errors.
			if (msd_ctx->diskops.SectorRead(msd_ctx->userp, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx), msd_ctx->LBA.Val, (uint8_t*)&msd_ctx->msd_buffer[0]) != true) {
				if (msd_ctx->MSDRetryAttempt < MSD_FAILED_READ_MAX_ATTEMPTS) {
					msd_ctx->MSDRetryAttempt++;
					break;
				} else {
					//Too many consecutive failed reads have occurred.  Need to
					//give up and abandon the sector read attempt; something must
					//be wrong and we don't want to get stuck in an infinite loop.
					//Need to indicate to the host that a device error occurred.
					//However, we can't send the CSW immediately, since the host
					//still expects to receive sector read data on the IN endpoint
					//first.  Therefore, we still send dummy bytes, before
					//we send the CSW with the failed status in it.
					msd_ctx->msd_csw.bCSWStatus=0x02;		// Indicate phase error 0x02
					// (option #1 from BOT section 6.6.2)
					//Set error status sense keys, so the host can check them later
					//to determine how to proceed.
					msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].SenseKey=S_MEDIUM_ERROR;
					msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASC=ASC_NO_ADDITIONAL_SENSE_INFORMATION;
					msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASCQ=ASCQ_NO_ADDITIONAL_SENSE_INFORMATION;
					USBStallEndpoint(msd_ctx->USB_EP_IN, IN_TO_HOST);
					msd_ctx->MSDReadState = MSD_READ10_WAIT;
					break;
				}
			}//else we successfully read a sector worth of data from our media

			msd_ctx->LBA.Val++;
			msd_ctx->msd_csw.dCSWDataResidue=BLOCKLEN_512;//in order to send the
			//512 bytes of data read

			msd_ctx->ptrNextData=(uint8_t *)&msd_ctx->msd_buffer[0];

			msd_ctx->MSDReadState = MSD_READ10_TX_SECTOR;
			//Fall through to MSD_READ10_TX_SECTOR

		case MSD_READ10_TX_SECTOR:
			if(msd_ctx->msd_csw.dCSWDataResidue == 0) {
				msd_ctx->MSDReadState = MSD_READ10_BLOCK;
				break;
			}

			msd_ctx->MSDReadState = MSD_READ10_TX_PACKET;
			//Fall through to MSD_READ10_TX_PACKET

		case MSD_READ10_TX_PACKET:
			/* Write next chunk of data to EP Buffer and send */

			//Make sure the endpoint is available before using it.
			if (USBHandleBusy(msd_ctx->USBMSDInHandle)) {
				break;
			}

			//Prepare the USB module to send an IN transaction worth of data to the host.
			msd_ctx->USBMSDInHandle = USBTxOnePacket(msd_ctx->USB_EP_IN, msd_ctx->ptrNextData, MSD_IN_EP_SIZE);
			msd_ctx->MSDReadState = MSD_READ10_TX_SECTOR;

			msd_ctx->gblCBW.dCBWDataTransferLength -= MSD_IN_EP_SIZE;
			msd_ctx->msd_csw.dCSWDataResidue -= MSD_IN_EP_SIZE;
			msd_ctx->ptrNextData += MSD_IN_EP_SIZE;
			break;

		default:
			//Illegal condition, should never occur.  In the event that it ever
			//did occur anyway, try to notify the host of the error.
			msd_ctx->msd_csw.bCSWStatus=0x02;  //indicate "Phase Error"
			USBStallEndpoint(msd_ctx->USB_EP_IN, IN_TO_HOST);
			//Advance state machine
			msd_ctx->MSDReadState = MSD_READ10_WAIT;
			break;
	}//switch(MSDReadState)

	return msd_ctx->MSDReadState;
}


uint8_t MSDWriteHandler(USBDeluxeDevice_MSDContext *msd_ctx) {
	switch (msd_ctx->MSDWriteState) {
		case MSD_WRITE10_WAIT:
			/* Read the LBA, TransferLength fields from Command Block
			NOTE: CB is Big-Endian */
			msd_ctx->LBA.v[3]=msd_ctx->gblCBW.CBWCB[2];
			msd_ctx->LBA.v[2]=msd_ctx->gblCBW.CBWCB[3];
			msd_ctx->LBA.v[1]=msd_ctx->gblCBW.CBWCB[4];
			msd_ctx->LBA.v[0]=msd_ctx->gblCBW.CBWCB[5];
			msd_ctx->TransferLength.v[1]=msd_ctx->gblCBW.CBWCB[7];
			msd_ctx->TransferLength.v[0]=msd_ctx->gblCBW.CBWCB[8];

			//Do some error case checking.
			if (MSDCheckForErrorCases(msd_ctx, msd_ctx->TransferLength.Val * (uint32_t)FILEIO_CONFIG_MEDIA_SECTOR_SIZE) != MSD_ERROR_CASE_NO_ERROR) {
				//An error was detected.  The MSDCheckForErrorCases() function will
				//have taken care of setting the proper states to report the error to the host.
				break;
			}

			//Check if the media is write protected before deciding what
			//to do with the data.
			if (msd_ctx->diskops.WriteProtectState(msd_ctx->userp, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx))) {
				//The media appears to be write protected.
				//Let host know error occurred.  The bCSWStatus flag is also used by
				//the write handler, to know not to even attempt the write sequence.
				msd_ctx->msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;

				//Set sense keys so the host knows what caused the error.
				msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].SenseKey=S_DATA_PROTECT;
				msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASC=ASC_WRITE_PROTECTED;
				msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASCQ=ASCQ_WRITE_PROTECTED;

				//Stall the OUT endpoint, so as to promptly inform the host
				//that the data cannot be accepted, due to write protected media.
				USBStallEndpoint(msd_ctx->USB_EP_OUT, OUT_FROM_HOST);
				msd_ctx->MSDWriteState = MSD_WRITE10_WAIT;
				return msd_ctx->MSDWriteState;
			}

			msd_ctx->MSD_State = MSD_WRITE10_BLOCK;
			//Fall through to MSD_WRITE10_BLOCK

		case MSD_WRITE10_BLOCK:
			if (msd_ctx->TransferLength.Val == 0) {
				msd_ctx->MSDWriteState = MSD_WRITE10_WAIT;
				break;
			}

			msd_ctx->MSDWriteState = MSD_WRITE10_RX_SECTOR;
			msd_ctx->ptrNextData=(uint8_t *)&msd_ctx->msd_buffer[0];

			msd_ctx->msd_csw.dCSWDataResidue=BLOCKLEN_512;

			//Fall through to MSD_WRITE10_RX_SECTOR
		case MSD_WRITE10_RX_SECTOR: {
			/* Read 512B into msd_buffer*/
			if(msd_ctx->msd_csw.dCSWDataResidue>0) {
				if (USBHandleBusy(msd_ctx->USBMSDOutHandle) == true) {
					break;
				}

				msd_ctx->USBMSDOutHandle = USBRxOnePacket(msd_ctx->USB_EP_OUT, msd_ctx->ptrNextData, MSD_OUT_EP_SIZE);
				msd_ctx->MSDWriteState = MSD_WRITE10_RX_PACKET;
				//Fall through to MSD_WRITE10_RX_PACKET
			} else {
				//We finished receiving a sector worth of data from the host.
				//Check if the media is write protected before deciding what
				//to do with the data.
				if(msd_ctx->diskops.WriteProtectState(msd_ctx->userp, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx))) {
					//The device appears to be write protected.
					//Let host know error occurred.  The bCSWStatus flag is also used by
					//the write handler, to know not to even attempt the write sequence.
					msd_ctx->msd_csw.bCSWStatus=0x01;

					//Set sense keys so the host knows what caused the error.
					msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].SenseKey=S_NOT_READY;
					msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASC=ASC_WRITE_PROTECTED;
					msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASCQ=ASCQ_WRITE_PROTECTED;
				}

				msd_ctx->MSDWriteState = MSD_WRITE10_SECTOR;
				break;
			}
		}
			//Fall through to MSD_WRITE10_RX_PACKET
		case MSD_WRITE10_RX_PACKET:
			if (USBHandleBusy(msd_ctx->USBMSDOutHandle) == true) {
				break;
			}

			msd_ctx->gblCBW.dCBWDataTransferLength -= USBHandleGetLength(msd_ctx->USBMSDOutHandle);		// 64B read
			msd_ctx->msd_csw.dCSWDataResidue -= USBHandleGetLength(msd_ctx->USBMSDOutHandle);
			msd_ctx->ptrNextData += MSD_OUT_EP_SIZE;

			msd_ctx->MSDWriteState = MSD_WRITE10_RX_SECTOR;
			break;

		case MSD_WRITE10_SECTOR: {
			//Make sure that no error has been detected, before performing the write
			//operation.  If there was an error, skip the write operation, but allow
			//the TransferLength to continue decrementing, so that we can eventually
			//receive all OUT bytes that the host is planning on sending us.  Only
			//after that is complete will the host send the IN token for the CSW packet,
			//which will contain the bCSWStatus letting it know an error occurred.
			if(msd_ctx->msd_csw.bCSWStatus == 0x00) {
				if(msd_ctx->diskops.SectorWrite(msd_ctx->userp, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx), msd_ctx->LBA.Val, (uint8_t*)&msd_ctx->msd_buffer[0], (msd_ctx->LBA.Val==0)?true:false) != true) {
					//The write operation failed for some reason.  Keep track of retry
					//attempts and abort if repeated write attempts also fail.
					if(msd_ctx->MSDRetryAttempt < MSD_FAILED_WRITE_MAX_ATTEMPTS) {
						msd_ctx->MSDRetryAttempt++;
						break;
					} else {
						//Too many consecutive failed write attempts have occurred.
						//Need to give up and abandon the write attempt.
						msd_ctx->msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED; //Indicate error during CSW phase
						//Set error status sense keys, so the host can check them later
						//to determine how to proceed.
						msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].SenseKey=S_MEDIUM_ERROR;
						msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASC=ASC_NO_ADDITIONAL_SENSE_INFORMATION;
						msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASCQ=ASCQ_NO_ADDITIONAL_SENSE_INFORMATION;
					}
				}
			}

			//One LBA is written (unless an error occurred).  Advance state
			//variables so we can eventually finish handling the CBW request.
			msd_ctx->LBA.Val++;
			msd_ctx->TransferLength.Val--;
			msd_ctx->MSDWriteState = MSD_WRITE10_BLOCK;
			break;
		}

		default:
			//Illegal condition which should not occur.  If for some reason it
			//does, try to let the host know know an error has occurred.
			msd_ctx->msd_csw.bCSWStatus = 0x02;    //Phase Error
			USBStallEndpoint(msd_ctx->USB_EP_OUT, OUT_FROM_HOST);
			msd_ctx->MSDWriteState = MSD_WRITE10_WAIT;
			break;
	}

	return msd_ctx->MSDWriteState;
}



void USBDeluxeDevice_MSD_ProcessCommandMediaPresent(USBDeluxeDevice_MSDContext *msd_ctx) {
	uint8_t i;
	uint8_t NumBytesInPacket;

	//Check what command we are currently processing, to decide how to handle it.
	switch (msd_ctx->MSDCommandState) {
		case MSD_READ_10:
			//The host issues a "Read 10" request when it wants to read some number
			//of 10-bit length blocks (512 byte blocks) of data from the media.
			//Since this is a common request and is part of the "critical path"
			//performance wise, we put this at the top of the state machine checks.
			if (MSDReadHandler(msd_ctx) == MSD_READ10_WAIT) {
				msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
			}
			break;

		case MSD_WRITE_10:
			//The host issues a "Write 10" request when it wants to write some number
			//of 10-bit length blocks (512 byte blocks) of data to the media.
			//Since this is a common request and is part of the "critical path"
			//performance wise, we put this near the top of the state machine checks.
			if (MSDWriteHandler(msd_ctx) == MSD_WRITE10_WAIT) {
				msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
			}
			break;

		case MSD_INQUIRY: {
			//The host wants to learn more about our MSD device (spec version,
			//supported abilities, etc.)

			//Error check: If host doesn't want any data, then just advance to CSW phase.
			if (msd_ctx->MSDHostNoData == true) {
				msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
				break;
			}

			//Get the 16-bit "Allocation Length" (Number of bytes to respond
			//with.  Note: Value provided in CBWCB is in big endian format)
			msd_ctx->TransferLength.byte.HB = msd_ctx->gblCBW.CBWCB[3]; //MSB
			msd_ctx->TransferLength.byte.LB = msd_ctx->gblCBW.CBWCB[4]; //LSB
			//Check for possible errors.
			if (MSDCheckForErrorCases(msd_ctx, msd_ctx->TransferLength.Val) != MSD_ERROR_CASE_NO_ERROR) {
				break;
			}

			//Compute and load proper csw residue and device in number of byte.
			MSDComputeDeviceInAndResidue(msd_ctx, sizeof(USBDeluxeDevice_MSD_InquiryResponse));

			//If we get to here, this implies no errors were found and the command is legit.

			//copy the inquiry results from the defined const buffer
			//  into the USB buffer so that it can be transmitted

			USBDeluxeDevice_MSD_InquiryResponse inq_resp_buf;
			memcpy(&inq_resp_buf, &msd_inq_resp, sizeof(USBDeluxeDevice_MSD_InquiryResponse));

#ifndef __dsPIC30__
#error The code piece below is PIC24/dsPIC33 specific
#endif
			if ((uint16_t)msd_ctx->vendor_id > 32767) {
				strncpy(inq_resp_buf.vendorID, msd_ctx->vendor_id, 8);
			}

			if ((uint16_t)msd_ctx->product_id > 32767) {
				strncpy(inq_resp_buf.productID, msd_ctx->product_id, 16);
			}

			if ((uint16_t)msd_ctx->product_rev > 32767) {
				strncpy(inq_resp_buf.productRev, msd_ctx->product_rev, 4);
			}

			memcpy(&msd_ctx->msd_buffer[0], &inq_resp_buf, sizeof(USBDeluxeDevice_MSD_InquiryResponse));	//Inquiry response is 36 bytes total
			msd_ctx->MSDCommandState = MSD_COMMAND_RESPONSE;
			break;
		}
		case MSD_READ_CAPACITY: {
			//The host asked for the total capacity of the device.  The response
			//packet is 8-bytes (32-bits for last LBA implemented, 32-bits for block size).
			USBDeluxeDevice_MSD_SECTOR_SIZE sectorSize;
			USBDeluxeDevice_MSD_CAPACITY capacity;

			//get the information from the physical media
			capacity.Val = msd_ctx->diskops.ReadCapacity(msd_ctx->userp, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx));
			sectorSize.Val = msd_ctx->diskops.ReadSectorSize(msd_ctx->userp, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx));

			// Reimu 2021-5-17:
			// It's funny that lots of commercial products with a PIC inside have this "one more sector" mistake.
			if (capacity.Val) {
				capacity.Val--;
			}

			//Copy the data to the buffer.  Host expects the response in big endian format.
			msd_ctx->msd_buffer[0]=capacity.v[3];
			msd_ctx->msd_buffer[1]=capacity.v[2];
			msd_ctx->msd_buffer[2]=capacity.v[1];
			msd_ctx->msd_buffer[3]=capacity.v[0];

			msd_ctx->msd_buffer[4]=sectorSize.v[3];
			msd_ctx->msd_buffer[5]=sectorSize.v[2];
			msd_ctx->msd_buffer[6]=sectorSize.v[1];
			msd_ctx->msd_buffer[7]=sectorSize.v[0];

			//Compute and load proper csw residue and device in number of byte.
			msd_ctx->TransferLength.Val = 0x08;      //READ_CAPACITY always has an 8-byte response.
			MSDComputeDeviceInAndResidue(msd_ctx, 0x08);

			msd_ctx->MSDCommandState = MSD_COMMAND_RESPONSE;
			break;
		}
		case MSD_REQUEST_SENSE:
			//The host normally sends this request after a CSW completed, where
			//the device indicated some kind of error on the previous transfer.
			//In this case, the host will typically issue this request, so it can
			//learn more details about the cause/source of the error condition.

			//Error check: if the host doesn't want any data, just advance to CSW phase.
			if(msd_ctx->MSDHostNoData == true) {
				msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
				break;
			}

			//Compute and load proper csw residue and device in number of byte.
			msd_ctx->TransferLength.Val = sizeof(USBDeluxeDevice_MSD_RequestSenseResponse);      //REQUEST_SENSE has an 18-byte response.
			MSDComputeDeviceInAndResidue(msd_ctx, sizeof(USBDeluxeDevice_MSD_RequestSenseResponse));

			//Copy the requested response data from flash to the USB ram buffer.
			for (i=0;i<sizeof(USBDeluxeDevice_MSD_RequestSenseResponse);i++) {
				msd_ctx->msd_buffer[i]=msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)]._byte[i];
			}
			msd_ctx->MSDCommandState = MSD_COMMAND_RESPONSE;
			break;

		case MSD_MODE_SENSE:
			msd_ctx->msd_buffer[0]=0x03;
			msd_ctx->msd_buffer[1]=0x00;
			msd_ctx->msd_buffer[2]=(msd_ctx->diskops.WriteProtectState(msd_ctx->userp, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx))) ? 0x80 : 0x00;
			msd_ctx->msd_buffer[3]= 0x00;

			//Compute and load proper csw residue and device in number of byte.
			msd_ctx->TransferLength.Val = 0x04;
			MSDComputeDeviceInAndResidue(msd_ctx, 0x04);
			msd_ctx->MSDCommandState = MSD_COMMAND_RESPONSE;
			break;

		case MSD_PREVENT_ALLOW_MEDIUM_REMOVAL:
			msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].SenseKey=S_ILLEGAL_REQUEST;
			msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASC=ASC_INVALID_COMMAND_OPCODE;
			msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASCQ=ASCQ_INVALID_COMMAND_OPCODE;
			msd_ctx->msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;
			msd_ctx->msd_csw.dCSWDataResidue = 0x00;
			msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
			break;

		case MSD_TEST_UNIT_READY:
			//The host will typically send this command periodically to check if
			//it is ready to be used and to obtain polled notification of changes
			//in status (ex: user removed media from a removable media MSD volume).
			//There is no data stage for this request.  The information we send to
			//the host in response to this request is entirely contained in the CSW.

			//First check for possible errors.
			if(MSDCheckForErrorCases(msd_ctx, 0) != MSD_ERROR_CASE_NO_ERROR)
			{
				break;
			}
			//The stack sets this condition when the status of the removable media
			//has just changed (ex: the user just plugged in the removable media,
			//in which case we want to notify the host of the changed status, by
			//sending a deliberate "error" notification).  This doesn't mean any
			//real error has occurred.
			if((msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].SenseKey==S_UNIT_ATTENTION) && (msd_ctx->msd_csw.bCSWStatus==MSD_CSW_COMMAND_FAILED))
			{
				msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
			}
			else
			{
				USBDeluxeDevice_MSD_ResetSenseData(msd_ctx, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx));
				msd_ctx->msd_csw.dCSWDataResidue=0x00;
				msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
			}
			break;

		case MSD_VERIFY:
			//Fall through to STOP_START

		case MSD_STOP_START:
			msd_ctx->msd_csw.dCSWDataResidue=0x00;
			msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
			break;

		case MSD_COMMAND_RESPONSE:
			//This command state didn't originate from the host.  This state was
			//set by the firmware (for one of the other handlers) when it was
			//finished preparing the data to send to the host, and it is now time
			//to transmit the data over the bulk IN endpoint.
			if (USBHandleBusy(msd_ctx->USBMSDInHandle) == false) {
				//We still have more bytes needing to be sent.  Compute how many
				//bytes should be in the next IN packet that we send.
				if (msd_ctx->gblCBW.dCBWDataTransferLength >= MSD_IN_EP_SIZE) {
					NumBytesInPacket = MSD_IN_EP_SIZE;
					msd_ctx->gblCBW.dCBWDataTransferLength -= MSD_IN_EP_SIZE;
				}
				else
				{
					//This is a short packet and will be our last IN packet sent
					//in the transfer.
					NumBytesInPacket = msd_ctx->gblCBW.dCBWDataTransferLength;
					msd_ctx->gblCBW.dCBWDataTransferLength = 0;
				}

				//We still have more bytes needing to be sent.  Check if we have
				//already fulfilled the device input expected quantity of bytes.
				//If so, we need to keep sending IN packets, but pad the extra
				//bytes with value = 0x00 (see error case 5 MSD device BOT v1.0
				//spec handling).
				if(msd_ctx->TransferLength.Val >= NumBytesInPacket) {
					//No problem, just send the requested data and keep track of remaining count.
					msd_ctx->TransferLength.Val -= NumBytesInPacket;
				} else {
					//The host is reading more bytes than the device has to send.
					//In this case, we still need to send the quantity of bytes requested,
					//but we have to fill the pad bytes with 0x00.  The below for loop
					//is execution speed inefficient, but performance isn't important
					//since this code only executes in the case of a host error
					//anyway (Hi > Di).
					for (i = 0; i < NumBytesInPacket; i++) {
						if (msd_ctx->TransferLength.Val != 0) {
							msd_ctx->TransferLength.Val--;
						} else {
							msd_ctx->msd_buffer[i] = 0x00;
						}
					}
				}

				//We are now ready to send the packet to the host.
				msd_ctx->USBMSDInHandle = USBTxOnePacket(msd_ctx->USB_EP_IN,(uint8_t*)&msd_ctx->msd_buffer[0], NumBytesInPacket);

				//Check to see if we are done sending all requested bytes of data
				if(msd_ctx->gblCBW.dCBWDataTransferLength == 0) {
					//We have sent all the requested bytes.  Go ahead and
					//advance state so as to send the CSW.
					msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
					break;
				}
			}
			break;
		case MSD_COMMAND_ERROR:
		default:
			//An unsupported command was received.  Since we are uncertain how many
			//bytes we should send/or receive, we should set sense key data and then
			//STALL, to force the host to perform error recovery.
			MSDErrorHandler(msd_ctx, MSD_ERROR_UNSUPPORTED_COMMAND);
			break;
	} // end switch
}//void MSDProcessCommandMediaPresent(void)


void USBDeluxeDevice_MSD_ProcessCommandMediaAbsent(USBDeluxeDevice_MSDContext *msd_ctx) {
	//Check what command we are currently processing, to decide how to handle it.
	switch (msd_ctx->MSDCommandState) {
		case MSD_REQUEST_SENSE:
			//The host sends this request when it wants to check the status of
			//the device, and/or identify the reason for the last error that was
			//reported by the device.
			//Set the sense keys to let the host know that the reason the last
			//command failed was because the media was not present.
			USBDeluxeDevice_MSD_ResetSenseData(msd_ctx, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx));
			msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].SenseKey = S_NOT_READY;
			msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASC = ASC_MEDIUM_NOT_PRESENT;
			msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASCQ = ASCQ_MEDIUM_NOT_PRESENT;

			//After initializing the sense keys above, the subsequent handling
			//code for this state is the same with or without media.
			//Therefore, to save code size, we just call the media present handler.
			USBDeluxeDevice_MSD_ProcessCommandMediaPresent(msd_ctx);
			break;

		case MSD_PREVENT_ALLOW_MEDIUM_REMOVAL:
		case MSD_TEST_UNIT_READY:
			//The host will typically periodically poll the device by sending this
			//request.  Since this is a removable media device, and the media isn't
			//present, we need to indicate an error to let the host know (to
			//check the sense keys, which will tell it the media isn't present).
			msd_ctx->msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;
			msd_ctx->MSDCommandState = MSD_COMMAND_WAIT;
			break;

		case MSD_INQUIRY:
			//The handling code for this state is the same with or without media.
			//Therefore, to save code size, we just call the media present handler.
			USBDeluxeDevice_MSD_ProcessCommandMediaPresent(msd_ctx);
			break;

		case MSD_COMMAND_RESPONSE:
			//The handling code for this state is the same with or without media.
			//Therefore, to save code size, we just call the media present handler.
			USBDeluxeDevice_MSD_ProcessCommandMediaPresent(msd_ctx);
			break;

		default:
			//An unsupported command was received.  Since we are uncertain how
			//many bytes we should send/or receive, we should set sense key data
			//and then STALL, to force the host to perform error recovery.
			MSDErrorHandler(msd_ctx, MSD_ERROR_UNSUPPORTED_COMMAND);
			break;
	}
}//void MSDProcessCommandMediaAbsent(void)

uint8_t USBDeluxeDevice_MSD_ProcessCommand(USBDeluxeDevice_MSDContext *msd_ctx) {
	//Check if the media is either not present, or has been flagged by firmware
	//to pretend to be non-present (ex: SoftDetached).

	//  || (msd_ctx->SoftDetach[msd_ctx->gblCBW.bCBWLUN] == true)
	if ((msd_ctx->diskops.MediaDetect(msd_ctx->userp, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)) == false)) {
		//Clear flag so we know the media need initialization, if it becomes
		//present in the future.
		msd_ctx->gblMediaPresent &= ~((uint16_t)1<<msd_ctx->gblCBW.bCBWLUN);
		USBDeluxeDevice_MSD_ProcessCommandMediaAbsent(msd_ctx);
	} else {
		//Check if the media is present and hasn't been already flagged as initialized.
		if ((msd_ctx->gblMediaPresent & ((uint16_t)1<<msd_ctx->gblCBW.bCBWLUN)) == 0) {
			//Try to initialize the media
			if (msd_ctx->diskops.MediaInitialize(msd_ctx->userp, USBDeluxeDevice_MSD_CurrentLUN(msd_ctx))) {
				//The media initialized successfully.  Set flag letting software
				//know that it doesn't need re-initialization again (unless the
				//media is removable and is subsequently removed and re-inserted).
				msd_ctx->gblMediaPresent |= ((uint16_t)1<<msd_ctx->gblCBW.bCBWLUN);

				//The media is present and has initialized successfully.  However,
				//we should still notify the host that the media may have changed,
				//from the host's perspective, since we just initialized it for
				//the first time.
				msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].SenseKey = S_UNIT_ATTENTION;
				msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASC = ASC_NOT_READY_TO_READY_CHANGE;
				msd_ctx->gblSenseData[USBDeluxeDevice_MSD_CurrentLUN(msd_ctx)].ASCQ = ASCQ_MEDIUM_MAY_HAVE_CHANGED;
				//Signify a soft error to the host, so it knows to check the
				//sense keys to learn that the media just changed.
				msd_ctx->msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED; //No real "error" per se has occurred
				//Process the command now.
				USBDeluxeDevice_MSD_ProcessCommandMediaPresent(msd_ctx);
			} else {
				//The media failed to initialize for some reason.
				USBDeluxeDevice_MSD_ProcessCommandMediaAbsent(msd_ctx);
			}
		} else {
			//The media was present and was already initialized/ready to process
			//the host's command.
			USBDeluxeDevice_MSD_ProcessCommandMediaPresent(msd_ctx);
		}
	}

	return msd_ctx->MSDCommandState;
}

uint8_t USBDeluxe_DeviceFunction_Add_MSD(void *userp, uint8_t nr_luns, const char *vendor_id, const char *product_id, const char *product_rev, USBDeluxeDevice_MSD_DiskOps *disk_ops) {
	uint8_t last_idx = usb_device_driver_ctx.size;

	uint8_t iface = USBDeluxe_DeviceDescriptor_InsertInterface(0, 2, MSD_INTF, MSD_INTF_SUBCLASS, MSD_PROTOCOL);
	uint8_t ep = USBDeluxe_DeviceDescriptor_InsertEndpoint(USB_EP_DIR_IN|USB_EP_DIR_OUT, _BULK, 64, 1, -1, -1);

	USBDeviceDriverContext *ctx = USBDeluxe_DeviceDriver_AllocateMemory(USB_FUNC_MSD);

	USBDeluxeDevice_MSD_Create(ctx->drv_ctx, userp, iface, ep, ep, nr_luns, vendor_id, product_id, product_rev, disk_ops);

	USBDeluxe_Device_TaskCreate(last_idx, "MSD");

	return last_idx;
}

#endif
