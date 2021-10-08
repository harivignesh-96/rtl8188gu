/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
/******************************************************************************
 *
 *
 * Module:	rtl8192c_rf6052.c	( Source C File)
 *
 * Note:	Provide RF 6052 series relative API.
 *
 * Function:
 *
 * Export:
 *
 * Abbrev:
 *
 * History:
 * Data			Who		Remark
 *
 * 09/25/2008	MHC		Create initial version.
 * 11/05/2008	MHC		Add API for tw power setting.
 *
 *
******************************************************************************/

#include <rtl8710b_hal.h>

/*------------------------Define local variable------------------------------*/
#ifdef CONFIG_RF_SHADOW_RW
/* 2008/11/20 MH For Debug only, RF
 * static	RF_SHADOW_T	RF_Shadow[RF6052_MAX_PATH][RF6052_MAX_REG] = {0}; */
static	RF_SHADOW_T	RF_Shadow[RF6052_MAX_PATH][RF6052_MAX_REG];
#endif /*CONFIG_RF_SHADOW_RW*/
/*------------------------Define local variable------------------------------*/

static VOID
phy_RF6052_Config_HardCode(
	IN	PADAPTER		Adapter
)
{

	/* Set Default Bandwidth to 20M */
	/* Adapter->HalFunc	.SetBWModeHandler(Adapter, CHANNEL_WIDTH_20); */

	/* TODO: Set Default Channel to channel one for RTL8225 */

}

static int
phy_RF6052_Config_ParaFile(
	IN	PADAPTER		Adapter
)
{
	u32					u4RegValue = 0;
	enum rf_path			eRFPath;
	BB_REGISTER_DEFINITION_T	*pPhyReg;

	int					rtStatus = _SUCCESS;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	/* 3//----------------------------------------------------------------- */
	/* 3// <2> Initialize RF */
	/* 3//----------------------------------------------------------------- */
	for (eRFPath = RF_PATH_A; eRFPath < pHalData->NumTotalRFPath; eRFPath++) {

		pPhyReg = &pHalData->PHYRegDef[eRFPath];

		/*----Store original RFENV control type----*/
		switch (eRFPath) {
		case RF_PATH_A:
			u4RegValue = phy_query_bb_reg(Adapter, pPhyReg->rfintfs, bRFSI_RFENV);
			break;
		default:
			RTW_ERR("Invalid rf_path:%d\n", eRFPath);
			break;
		}

		/*----Set RF_ENV enable----*/
		phy_set_bb_reg(Adapter, pPhyReg->rfintfe, bRFSI_RFENV << 16, 0x1);
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		/*----Set RF_ENV output high----*/
		phy_set_bb_reg(Adapter, pPhyReg->rfintfo, bRFSI_RFENV, 0x1);
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		/* Set bit number of Address and Data for RF register */
		phy_set_bb_reg(Adapter, pPhyReg->rfHSSIPara2, b3WireAddressLength, 0x0);	/* Set 1 to 4 bits for 8255 */
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		phy_set_bb_reg(Adapter, pPhyReg->rfHSSIPara2, b3WireDataLength, 0x0);	/* Set 0 to 12  bits for 8255 */
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		/*----Initialize RF fom connfiguration file----*/
		switch (eRFPath) {
		case RF_PATH_A:
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
			if (PHY_ConfigRFWithParaFile(Adapter, PHY_FILE_RADIO_A, eRFPath) == _FAIL)
#endif
			{
#ifdef CONFIG_EMBEDDED_FWIMG
				if (odm_config_rf_with_header_file(&pHalData->odmpriv, CONFIG_RF_RADIO, eRFPath) == HAL_STATUS_FAILURE)
					rtStatus = _FAIL;
#endif
			}
			break;
		default:
			RTW_ERR("Invalid rf_path:%d\n", eRFPath);
			break;
		}

		/*----Restore RFENV control type----*/;
		switch (eRFPath) {
		case RF_PATH_A:
			phy_set_bb_reg(Adapter, pPhyReg->rfintfs, bRFSI_RFENV, u4RegValue);
			break;
		default:
			RTW_ERR("Invalid rf_path:%d\n", eRFPath);
			break;
		}

		if (rtStatus != _SUCCESS) {
			goto phy_RF6052_Config_ParaFile_Fail;
		}

	}

	/* 3 ----------------------------------------------------------------- */
	/* 3 Configuration of Tx Power Tracking */
	/* 3 ----------------------------------------------------------------- */

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
	if (PHY_ConfigRFWithTxPwrTrackParaFile(Adapter, PHY_FILE_TXPWR_TRACK) == _FAIL)
#endif
	{
#ifdef CONFIG_EMBEDDED_FWIMG
		odm_config_rf_with_tx_pwr_track_header_file(&pHalData->odmpriv);
#endif
	}

	return rtStatus;

phy_RF6052_Config_ParaFile_Fail:
	return rtStatus;
}


int
PHY_RF6052_Config8710B(
	IN	PADAPTER		Adapter)
{
	HAL_DATA_TYPE				*pHalData = GET_HAL_DATA(Adapter);
	int					rtStatus = _SUCCESS;

	/* Initialize general global value */
	if (pHalData->rf_type == RF_1T1R)
		pHalData->NumTotalRFPath = 1;
	else
		pHalData->NumTotalRFPath = 2;

	/* Config BB and RF */
	rtStatus = phy_RF6052_Config_ParaFile(Adapter);
	return rtStatus;

}

/* End of HalRf6052.c */
