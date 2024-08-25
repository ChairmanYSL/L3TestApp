/**
************************************************************************************************************************
*                                        COPYRIGHT (C) 2014 Kingsee Tech. Co., Ltd.
*
* @file     emv_ics.h
* @brief   	EMVÏà¹Ø±êÇ©
* @author   Kingsee Development Team - chenjinhai@szzt.com.cn
* @version  V43D1.0.28
*
************************************************************************************************************************
*/
#ifndef EMV_TAG_H_
#define EMV_TAG_H_

/** [42] - Issuer Identification Number (IIN) | ·¢¿¨ÐÐÊ¶±ðÂë£¨IIN£©, a6, 3 */
#define TAG_0042_IIN   0X0042

/** Ó¦ÓÃ±êÊ¶·û-¿¨Æ¬|Application Dedicated File (ADF) Name[4F][b][5-16] */
#define TAG_004F_AID   0X004F

/** Ó¦ÓÃ±êÇ©|Application Label[50][ans1-16][1-16] */
#define TAG_0050_APPLICATION_LABEL   0X0050

/** [57] - Track2 Equivalent Data | ¶þ´ÅµÀµÈÐ§Êý¾Ý, b, var. up to 19 */
#define TAG_0057_TRACK2_EQUIVALENT_DATA   0X0057

/** [5A] - Application PAN | Ó¦ÓÃÖ÷ÕËºÅ£¨PAN£©, cn var. up to 19, var. up to 10 */
#define TAG_005A_APPLICATION_PAN   0X005A

/** [61] - Application Template */
#define TAG_0061_APPLICATION_TEMPLATE   0X0061

/** [6F] - FCI (FILE CONTROL INFO) Template */
#define TAG_006F_FCI_TEMPLATE   0X006F

/** [70] - AEF (Application Elementary File) Data Template */
#define TAG_0070_AEF_DATA_TEMPLATE   0X0070

/** [71] - Issuer Script Template 1 */
#define TAG_0071_ISSUER_SCRIPT_TEMPLATE_1   0X0071

/** [72] - Issuer Script Template 2 */
#define TAG_0072_ISSUER_SCRIPT_TEMPLATE_2   0X0072

/** [73] - Directory Discretionary Template */
#define TAG_0073_DIRECTORY_DISCRETIONARY_TEMPLATE   0X0073

/** [77] - Response Message Template Format 2 */
#define TAG_0077_RESPONSE_MESSAGE_TEMPLATE_FORMAT_2   0X0077

/** [80] - Response Message Template Format 1 */
#define TAG_0080_RESPONSE_MESSAGE_TEMPLATE_FORMAT_1   0X0080

/** [81] - Amount Authorised Binary */
#define TAG_0081_AMOUNT_AUTHORISED_BINARY   0X0081

/** [82] - AIP (Application Interchange Profile) */
#define TAG_0082_AIP   0X0082

/** [83] - Command Template */
#define TAG_0083_COMMAND_TEMPLATE   0X0083

/** [84] - DF (Dedicated File) Name */
#define TAG_0084_DF_NAME   0X0084

/** [86] - Issuer Script Command */
#define TAG_0086_ISSUER_SCRIPT_COMMAND   0X0086

/** Ó¦ÓÃÓÅÏÈÖ¸Ê¾Æ÷|Application Priority Indicator (API)[87][b8][1] */
#define TAG_0087_API   0X0087

/** [88] - SFI (Short File Indicator) */
#define TAG_0088_SFI   0X0088

/** [89] - Authorisation Code */
#define TAG_0089_AUTH_CODE   0X0089

/** [8A] - Authorisation Response Code (ARC) | ÊÚÈ¨ÏìÓ¦Âë, an2, 2 */
#define TAG_008A_ARC   0X008A

/** [8C] - CDOL 1 (Card Risk Management Data Object List) */
#define TAG_008C_CDOL_1   0X008C

/** [8D] - CDOL 2 (Card Risk Management Data Object List) */
#define TAG_008D_CDOL_2   0X008D

/** [8E] - CVM (Cardholder Verification Method) List */
#define TAG_008E_CVM_LIST   0X008E

/** [8F] - Certification Authority Public Key Index */
#define TAG_008F_CERTIFICATION_AUTHORITY_PUBLIC_KEY_INDEX   0X008F

/** [90] - Issuer Public Key Certificate */
#define TAG_0090_ISSUER_PUBLIC_KEY_CERTIFICATE   0X0090

/** [91] - Issuer Authentication Data */
#define TAG_0091_ISSUER_AUTHENTICATION_DATA   0X0091

/** [92] - Issuer Public Key Remainder */
#define TAG_0092_ISSUER_PUBLIC_KEY_REMAINDER   0X0092

/** [93] - Signed Static Application Data */
#define TAG_0093_SIGNED_STATIC_APPLICATION_DATA   0X0093

/** [94] - Application File Locator (AFL) | Ó¦ÓÃÎÄ¼þ¶¨Î»Æ÷, var., var. up to 252 */
#define TAG_0094_AFL   0X0094

/** [95] - TVR (Terminal Verification Results) */
#define TAG_0095_TVR   0X0095

/** [97] - TDOL (Transaction Certificate Data Object List) */
#define TAG_0097_TDOL   0X0097

/** [98] - TC (Transaction Certificate) Hash Value */
#define TAG_0098_TC_HASH_VALUE   0X0098

/** [99] - Transaction PIN (Personal Identification Number) Data */
#define TAG_0099_TRANSACTION_PIN_DATA   0X0099

/** [9A] - Transaction Date */
#define TAG_009A_TRANSACTION_DATE   0X009A

/** [9B] - Transaction Status Info */
#define TAG_009B_TSI   0X009B

/** [9C] - Transaction Type */
#define TAG_009C_TRANSACTION_TYPE   0X009C

/** [9D] - DDF (Directory Definition File) Name */
#define TAG_009D_DDF_NAME   0X009D

/** [A5] - FCI (File Control Info) Proprietary Template */
#define TAG_00A5_FCI_PROPRIETARY_TEMPLATE   0X00A5

/** [5F20] - Cardholder Name */
#define TAG_5F20_CARDHOLDER_NAME   0X5F20

/** [5F24] - Application Expiration Date */
#define TAG_5F24_APPLICATION_EXPIRATION_DATE   0X5F24

/** [5F25] - Application Effective Date */
#define TAG_5F25_APPLICATION_EFFECTIVE_DATE   0X5F25

/** [5F28] - Issuer Country Code */
#define TAG_5F28_ISSUER_COUNTRY_CODE   0X5F28

/** [5F2A] - Transaction Currency Code | ½»Ò×»õ±Ò´úÂë, n2, 1 */
#define TAG_5F2A_TRANSACTION_CURRENCY_CODE   0X5F2A

/** [5F2D] - Language Preference */
#define TAG_5F2D_LANGUAGE_PREFERENCE   0X5F2D

/** [5F30] - Service Code */
#define TAG_5F30_SERVICE_CODE   0X5F30

/** [5F34] - Application PAN Sequence | Ó¦ÓÃÖ÷ÕËºÅÐòÁÐºÅ, n2, 1 */
#define TAG_5F34_APPLICATION_PAN_SEQUENCE   0X5F34

/** [5F36] - Transaction Currency Exponent */
#define TAG_5F36_TRANSACTION_CURRENCY_EXPONENT   0X5F36

/** [5F50] - Issuer URL | ·¢¿¨ÐÐURL\n ans, var. */
#define TAG_5F50_ISSUER_URL   0X5F50

/** [5F57] - Account Type | ÕÊ»§ÀàÐÍ\n n2, 1 */
#define TAG_5F57_ACCOUNT_TYPE   0X5F57

/** [9F01] - Acquirer Identifier */
#define TAG_9F01_ACQUIRER_IDENTIFIER   0X9F01

/** [9F02] - Amount Authorised Numeric */
#define TAG_9F02_AMOUNT_AUTHORISED_NUMERIC   0X9F02

/** [9F03] - Amount Other Numeric */
#define TAG_9F03_AMOUNT_OTHER_NUMERIC   0X9F03

/** [9F04] - Amount Other Binary */
#define TAG_9F04_AMOUNT_OTHER_BINARY   0X9F04

/** [9F05] - Application Discretionary Data */
#define TAG_9F05_APPLICATION_DISCRETIONARY_DATA   0X9F05

/** [9F06] - AID (Terminal) */
#define TAG_9F06_AID_TERMINAL   0X9F06

/** [9F07] - AUC (Application Usage Control) */
#define TAG_9F07_AUC   0X9F07

/** [9F08] - Application Version Number Card */
#define TAG_9F08_APPLICATION_VERSION_NUMBER_CARD   0X9F08

/** [9F09] - Application Version Number Terminal */
#define TAG_9F09_APPLICATION_VERSION_NUMBER_TERMINAL   0X9F09

/** [9F0B] - Cardholder Name Extended */
#define TAG_9F0B_CARDHOLDER_NAME_EXTENDED   0X9F0B

/** [9F0D] - Issuer Action Code Default */
#define TAG_9F0D_ISSUER_ACTION_CODE_DEFAULT   0X9F0D

/** [9F0E] - Issuer Action Code Denial */
#define TAG_9F0E_ISSUER_ACTION_CODE_DENIAL   0X9F0E

/** [9F0F] - Issuer Action Code Online */
#define TAG_9F0F_ISSUER_ACTION_CODE_ONLINE   0X9F0F

/** [9F10] - Issuer Application Data (IAD) | ·¢¿¨ÐÐÓ¦ÓÃÊý¾Ý, b, var. up to 32 */
#define TAG_9F10_IAD   0X9F10

/** [9F11] - Issuer Code Table Index */
#define TAG_9F11_ISSUER_CODE_TABLE_INDEX   0X9F11

/** Ó¦ÓÃÊ×Ñ¡Ãû³Æ|Application Preferred Name[9F12][ans1-16][1-16] */
#define TAG_9F12_APPLICATION_PREFERRED_NAME   0X9F12

/** [9F13] - Last Online ATC (Application Transaction Counter) Register */
#define TAG_9F13_LAST_ONLINE_ATC_REGISTER   0X9F13

/** [9F14] - Lower Consecutive Offline Limit */
#define TAG_9F14_LOWER_CONSECUTIVE_OFFLINE_LIMIT   0X9F14

/** [9F15] - Merchant Category Code */
#define TAG_9F15_MERCHANT_CATEGORY_CODE   0X9F15

/** [9F16] - Merchant Identifier */
#define TAG_9F16_MERCHANT_IDENTIFIER   0X9F16

/** [9F17] - PIN (Personal Identification Number) Try Counter */
#define TAG_9F17_PIN_TRY_COUNTER   0X9F17

/** [9F18] - Issuer Script Identifier */
#define TAG_9F18_ISSUER_SCRIPT_IDENTIFIER   0X9F18

/** [9F1A] - Terminal Country Code */
#define TAG_9F1A_TERMINAL_COUNTRY_CODE   0X9F1A

/** [9F1B] - ÖÕ¶Ë×îµÍÏÞ¶î, b, 4 */
#define TAG_9F1B_TERMINAL_FLOOR_LIMIT   0X9F1B

/** [9F1C] - Terminal Identification */
#define TAG_9F1C_TERMINAL_IDENTIFICATION   0X9F1C

/** [9F1D] - Terminal Risk Mangement Data */
#define TAG_9F1D_TERMINAL_RISK_MANGEMENT_DATA   0X9F1D

/** [9F1E] - IFD (Interface Device) Serial Number */
#define TAG_9F1E_IFD_SERIAL_NUMBER   0X9F1E

/** [9F1F] - Track1 Discretionary Data */
#define TAG_9F1F_TRACK1_DISCRETIONARY_DATA   0X9F1F

/** [9F20] - Track2 Discretionary Data */
#define TAG_9F20_TRACK2_DISCRETIONARY_DATA   0X9F20

/** [9F21] - Transaction Time */
#define TAG_9F21_TRANSACTION_TIME   0X9F21

/** [9F22] - ÈÏÖ¤ÖÐÐÄ¹«Ô¿Ë÷Òý */
#define TAG_9F22_CAPK_INDEX   0X9F22

/** [9F23] - Upper Consecutive Offline Limit */
#define TAG_9F23_UPPER_CONSECUTIVE_OFFLINE_LIMIT   0X9F23

/** [9F24] - Payment Account Reference,an,29 */
#define TAG_9F24_PAYMENT_ACCOUNT_REFERENCE   0X9F24

/** [9F26] - Application Cryptogram */
#define TAG_9F26_APPLICATION_CRYPTOGRAM   0X9F26

/** [9F27] - Cryptogram Info Data */
#define TAG_9F27_CRYPTOGRAM_INFO_DATA   0X9F27

/** [9F2D] - Icc PIN Encipherment Public Key Certificate */
#define TAG_9F2D_ICC_PIN_ENCIPHERMENT_PUBLIC_KEY_CERTIFICATE   0X9F2D

/** [9F2E] - Icc PIN Encipherment Public Key Exponent */
#define TAG_9F2E_ICC_PIN_ENCIPHERMENT_PUBLIC_KEY_EXPONENT   0X9F2E

/** [9F2F] - Icc PIN Encipherment Public Key Remainder */
#define TAG_9F2F_ICC_PIN_ENCIPHERMENT_PUBLIC_KEY_REMAINDER   0X9F2F

/** [9F32] - Issuer Public Key Exponent */
#define TAG_9F32_ISSUER_PUBLIC_KEY_EXPONENT   0X9F32

/** [9F33] - Terminal Capabilities */
#define TAG_9F33_TERMINAL_CAPABILITIES   0X9F33

/** [9F34] - CVM (Cardholder Verification Method) Results */
#define TAG_9F34_CVM_RESULTS   0X9F34

/** [9F35] - Terminal Type */
#define TAG_9F35_TERMINAL_TYPE   0X9F35

/** [9F36] - ATC (Application Transaction Counter) */
#define TAG_9F36_ATC   0X9F36

/** [9F37] - Unpredicatable Number */
#define TAG_9F37_UNPREDICATABLE_NUMBER   0X9F37

/** [9F38] - PDOL (Processing Options Data Object List) */
#define TAG_9F38_PDOL   0X9F38

/** [9F39] - PoS (Point of Service) Entry Mode */
#define TAG_9F39_POS_ENTRY_MODE   0X9F39

/** [9F3A] - Amount Reference Currency */
#define TAG_9F3A_AMOUNT_REFERENCE_CURRENCY   0X9F3A

/** [9F3B] - Application Reference Currency */
#define TAG_9F3B_APPLICATION_REFERENCE_CURRENCY   0X9F3B

/** [9F3C] - Transaction Reference Currency */
#define TAG_9F3C_TRANSACTION_REFERENCE_CURRENCY   0X9F3C

/** [9F3D] - Transaction Reference Currency Exponent */
#define TAG_9F3D_TRANSACTION_REFERENCE_CURRENCY_EXPONENT   0X9F3D

/** [9F40] - Additional Terminal Capabilities */
#define TAG_9F40_ADDITIONAL_TERMINAL_CAPABILITIES   0X9F40

/** [9F41] - Transaction Sequence Counter */
#define TAG_9F41_TRANSACTION_SEQUENCE_COUNTER   0X9F41

/** [9F42] - Application Currency Code */
#define TAG_9F42_APPLICATION_CURRENCY_CODE   0X9F42

/** [9F43] - Application Reference Currency Exponent */
#define TAG_9F43_APPLICATION_REFERENCE_CURRENCY_EXPONENT   0X9F43

/** [9F44] - Application Currency Exponent */
#define TAG_9F44_APPLICATION_CURRENCY_EXPONENT   0X9F44

/** [9F45] - Data Authentication Code */
#define TAG_9F45_DATA_AUTHENTICATION_CODE   0X9F45

/** [9F46] - ICC Public Key Certificate */
#define TAG_9F46_ICC_PUBLIC_KEY_CERTIFICATE   0X9F46

/** [9F47] - ICC Public Key Exponent */
#define TAG_9F47_ICC_PUBLIC_KEY_EXPONENT   0X9F47

/** [9F48] - ICC Public Key Remainder */
#define TAG_9F48_ICC_PUBLIC_KEY_REMAINDER   0X9F48

/** [9F49] - DDOL (Dynamic Data Object List) */
#define TAG_9F49_DDOL   0X9F49

/** [9F4A] - SDA (Static Data Authentication) Tag List */
#define TAG_9F4A_SDA_TAG_LIST   0X9F4A

/** [9F4B] - Signed Dynamic Application Data */
#define TAG_9F4B_SIGNED_DYNAMIC_APPLICATION_DATA   0X9F4B

/** [9F4C] - ICC Dynamic Number */
#define TAG_9F4C_ICC_DYNAMIC_NUMBER   0X9F4C

/** [9F4D] - ½»Ò×ÈÕÖ¾Èë¿Ú */
#define TAG_9F4D_LOG_SFI   0X9F4D

/** [9F4E] - Merchant Name and Location */
#define TAG_9F4E_MERCHANT_NAME   0X9F4E

/** [9F4F] - ½»Ò×ÈÕÖ¾¸ñÊ½ */
#define TAG_9F4F_LOG_FORMAT 0X9F4F

/** [9F51] - Application Currency Code | Ó¦ÓÃ»õ±Ò´úÂë, n4, 2 */
#define TAG_9F51_APPLICATION_CURRENCY_CODE   0X9F51

/** [9F53] -Transaction Category Code, an, 1 */
#define TAG_9F53_TRANSACITON_CATEGORY_CODE   0X9F53

/** [9F5D] - Available Offline Spending Amount (AOSA) | ¿ÉÓÃÍÑ»úÏû·Ñ½ð¶î, n12, 6 */
#define TAG_9F5D_AVAILABLE_OFFLINE_SPENDING_AMOUNT   0X9F5D

/** [9F61] - ³Ö¿¨ÈËÖ¤¼þºÅ, an40, 1-40 */
#define TAG_9F61_CARDHOLDER_DOCUMENTS_NUMBER   0X9F61

/** [9F62] - ³Ö¿¨ÈËÖ¤¼þÀàÐÍ, ¶¨Òå¿É²Î¿¼{@link PBOC_DocumentTypes}, cn1, 1 */
#define TAG_9F62_CARDHOLDER_DOCUMENTS_TYPE   0X9F62

/** [9F63] - ²úÆ·±êÊ¶ÐÅÏ¢, b, 16 */
#define TAG_9F63_PRODUCT_IDENTIFICATION_INFORMATION   0X9F63

/** [9F66] - Terminal Transaction Qualifiers (TTQ) | ÖÕ¶Ë½»Ò×ÊôÐÔ */
#define TAG_9F66_TERMINAL_TRANSACTION_QUALIFIERS   0X9F66

/** [9F69] - Card Authentication Related Data | ¿¨Æ¬ÈÏÖ¤Ïà¹ØÊý¾Ý  */
#define TAG_9F69_CARD_AUTHENTICATION_RELATED_DATA   0X9F69

/** [9F6C] - Card Transaction Qualifiers (CTQ) | ¿¨Æ¬½»Ò×ÊôÐÔ */
#define TAG_9F6C_CARD_TRANSACTION_QUALIFIERS   0X9F6C

/** [9F6D] - EC Reset Threshold | µç×ÓÏÖ½ðÖØÖÃãÐÖµ, n12, 6 */
#define TAG_9F6D_EC_RESET_THRESHOLD   0X9F6D

/** [9F6D] - Mag-stripe Application Version Number (Reader)Öµ, b,2,k */
#define TAG_9F6D_MSD_APP_VER_READER   0X9F6D

/** [9F6E] - Third Party DataÖµ, b,5-32 */
#define TAG_9F6E_THIRD_PARTY_DATA   0X9F6E

/** [9F74] - EC Issuer Authorization Code | µç×ÓÏÖ½ð·¢¿¨ÐÐÊÚÈ¨Âë, a, 6 */
#define TAG_9F74_EC_ISSUER_AUTHORIZATION_CODE   0X9F74

/** [9F77] - EC Balance Limit | µç×ÓÏÖ½ðÓà¶îÉÏÏÞ, n12, 6 */
#define TAG_9F77_EC_BALANCE_LIMIT   0X9F77

/** [9F78] - EC Single Transaction Limit | µç×ÓÏÖ½ðµ¥±Ê½»Ò×ÏÞ¶î, n12, 6 */
#define TAG_9F78_EC_SINGLE_TRANSACTION_LIMIT   0X9F78

/** [9F79] - EC Balance | µç×ÓÏÖ½ðÓà¶î, n12, 6 */
#define TAG_9F79_EC_BALANCE   0X9F79

/** [9F7A] - EC Terminal Support Indicator , b, 1 */
#define TAG_9F7A_EC_TERMINAL_SUPPORT_INDICATOR   0X9F7A

/** [9F7B] - EC Terminal Transaction Limit , n12, 6 */
#define TAG_9F7B_EC_TERMINAL_TRANSACTION_LIMIT   0X9F7B

/** [9F7C] - Merchant Custom Data, b,20,k */
#define TAG_9F7C_MECHANT_CUSTOM_DATA   0X9F7C

/** [9F7E] - Mobile Support Indicator, b,1,k */
#define TAG_9F7E_MOBILE_SUP_INDICATOR   0X9F7E

/** [BF0C] - FCI (File Control Info) Issuer Discretionary Data */
#define TAG_BF0C_FCI_ISSUER_DISCRETIONARY_DATA   0XBF0C

/** [DF01] - Ó¦ÓÃÑ¡ÔñÖ¸Ê¾Æ÷£¨ASI£©, b, 1 */
#define TAG_DF01_ASI   0XDF01

/** [DF02] - ÈÏÖ¤ÖÐÐÄ¹«Ô¿Ä£, b, 248 */
#define TAG_DF02_CAPK_MODULUS   0XDF02

/** [DF03] - ÈÏÖ¤ÖÐÐÄ¹«Ô¿Ð£ÑéÖµ, b, ??? */
#define TAG_DF03_CAPK_CHECKSUM   0XDF03

/** [DF04] - ÈÏÖ¤ÖÐÐÄ¹«Ô¿Ö¸Êý, b, 1 or 3 */
#define TAG_DF04_CAPK_EXPONENT   0XDF04

/** [DF05] - ÈÏÖ¤ÖÐÐÄ¹«Ô¿ÓÐÐ§ÆÚ, n8, 8 */
#define TAG_DF05_CAPK_EXPIRATION_DATE   0XDF05

/** [DF06] - ÈÏÖ¤ÖÐÐÄ¹«Ô¿¹þÏ£Ëã·¨±êÊ¶, b, 1 */
#define TAG_DF06_CAPK_HASH_ALGORITHM_INDICATOR   0XDF06

/** [DF07] - ÈÏÖ¤ÖÐÐÄ¹«Ô¿Ëã·¨±êÊ¶, b, 1 */
#define TAG_DF07_CAPK_ALGORITHM_INDICATOR   0XDF07

/** [DF11] - ÖÕ¶ËÐÐÎª´úÂë-È±Ê¡, b, 5 */
#define TAG_DF11_TAC_DEFAULT   0XDF11

/** [DF12] - ÖÕ¶ËÐÐÎª´úÂë-Áª»ú, b, 5 */
#define TAG_DF12_TAC_ONLINE   0XDF12

/** [DF13] - ÖÕ¶ËÐÐÎª´úÂë-¾Ü¾ø, b, 5 */
#define TAG_DF13_TAC_DENIAL   0XDF13

/** [DF14] - È±Ê¡DDOL */
#define TAG_DF14_DEFAULT_DDOL   0XDF14

/** [DF15] - Æ«ÖÃËæ»úÑ¡Ôñ·§Öµ, b, 4 */
#define TAG_DF15_THRESHOLD_VALUE   0XDF15

/** [DF16] - Æ«ÖÃËæ»úÑ¡Ôñ×î´óÄ¿±ê°Ù·ÖÊý, b, 1 FXIME */
#define TAG_DF16_MAX_TARGET_PERCENTAGE   0XDF16

/** [DF17] - Ëæ»úÑ¡ÔñÄ¿±ê°Ù·ÖÊý, b, 1 FXIME */
#define TAG_DF17_TARGET_PERCENTAGE   0XDF17

/** [DF18] - ÖÕ¶ËÁª»úPINÖ§³ÖÄÜÁ¦, b, 1 */
#define TAG_DF18_ONLINE_PIN_SUPPORT_INDICATOR   0XDF18

/** [DF19] - ·Ç½Ó´¥ÍÑ»ú×îµÍÏÞ¶î, cn, 6 */
#define TAG_DF19_CONTACTLESS_FLOOR_LIMIT   0XDF19

/** [DF20] - ·Ç½Ó´¥½»Ò×ÏÞ¶î, cn, 6 */
#define TAG_DF20_CONTACTLESS_TRANSACTION_LIMIT   0XDF20

/** [DF21] - ³Ö¿¨ÈËÑéÖ¤·½·¨£¨CVM£©ËùÐèÏÞ¶î, cn, 6 */
#define TAG_DF21_CVM_REQUIRED_LIMIT   0XDF21

/** [DF31] - ·¢¿¨ÐÐ½Å±¾´¦Àí½á¹û */
#define TAG_DF31_ISSUER_SCRIPT_RESULTS   0XDF31

/** [DF4D] - È¦´æÈÕÖ¾Èë¿Ú, b16, 2 */
#define TAG_DF4D_LOAD_LOG_SFI   0XDF4D

/** [DF4D] - È¦´æÈÕÖ¾¸ñÊ½, b, var. */
#define TAG_DF4F_LOAD_LOG_FORMAT   0XDF4F

/** [DF60] - DS Input (Card), b,8,k/act/det. */
#define TAG_DF60_DS_INPUT_CARD   0XDF60

/** [DF62] - DS ODS Info, b,1,k/act/det. */
#define TAG_DF62_DS_ODS_INFO   0XDF62

/** [DF63] -DS ODS Term, b,var up tp 160,k/act/det. */
#define TAG_DF63_DS_ODS_TERM   0XDF63

/** [DF69] - SMËã·¨Ö§³ÖÖ¸Ê¾Æ÷, b8, 1 */
#define TAG_DF69_SM_SUPPORT_INDICATOR   0XDF69

/** [DF70] - ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö§ï¿½ï¿½Ö¸Ê¾ï¿½ï¿½ */
#define TAG_DF70_EXCEPTION_FILE_SUPPORT 0xDF70

//**[DF8101] DS Summary 2
#define TAG_DF8101_DS_SUMMARY2	0xDF8101

//**[DF8102] DS Summary 3
#define TAG_DF8102_DS_SUMMARY3	0xDF8102

//**[DF8104] Balance before GAC
#define TAG_DF8104_BALANCE_BEFOREGAC	0xDF8104

//**[DF8105] Balance after GAC
#define TAG_DF8105_BALANCE_AFTERGAC	0xDF8105

//**[DF8106] Data Needed
#define TAG_DF8106_DATA_NEEDED	0xDF8106

//**[DF8107]CDOL1 Related Data
#define TAG_DF8107_CDOL1_RELATED_DATA	0xDF8107

//**[DF8108]DS AC Type
#define TAG_DF8108_DS_AC_TYPE	0xDF8108

//**[DF8109]DS Input (Term)
#define TAG_DF8109_DS_INPUT_TERM	0xDF8109

//**[DF810A]DS ODS Info For Reader
#define TAG_DF810A_DS_INPUT_FOR_READER	0xDF810A

//**[DF810B]DS Summary Status
#define TAG_DF810B_DS_SUMMARY_STATUS 0xDF810B

//**[DF810C]Kernel ID
#define TAG_DF810C_KERNEL_ID 0xDF810C

//**[DF810D]DSVN Term || Var b k
#define TAG_DF810D_DSVN_TERM 0xDF810D

//**[DF810E]Post-Gen AC Put Data Status
#define TAG_DF810E_POSTGAC_PUTDATA_STATUS 0xDF810E

//**[DF810F]Post-Gen AC Put Data Status
#define TAG_DF810F_PREGAC_PUTDATA_STATUS 0xDF810F

//**[DF8111] PDOL Related Data
#define TAG_DF8111_PDOL_RELATED_DATA 0xDF8111

//**[DF8112] Tags To Read
#define TAG_DF8112_TAGS_TO_READ	0xDF8112

//**[DF8113] Tags To Read Yet
#define TAG_DF8113_TAGS_TO_READ_YET	0xDF8113

//**[DF8114] Reference Control Parameter
#define TAG_DF8114_REF_CONTROL_PARAM	0xDF8114

//**[DF8115] Error Indication
#define TAG_DF8115_ERROR_INDICATION	0xDF8115

//**[DF8116] User Interface Request Data
#define TAG_DF8116_UI_REQUEST_DATA	0xDF8116

//**[DF8117] Card Data Input Capability
#define TAG_DF8117_CARDDATA_INPUT_CAP	0xDF8117

//**[DF8118] CVM Capability ï¿½?CVM Required
#define TAG_DF8118_CVMCAP_CVMREQ	0xDF8118

//**[DF8119] CVM Capability ï¿½?No CVM Required
#define TAG_DF8119_CVMCAP_NOCVM	0xDF8119

//**[DF811A] Default UDOL
#define TAG_DF811A_DEFAULT_UDOL	0xDF811A

//**[DF811B] Kernel Configuration
#define TAG_DF811B_KERNEL_CONFIG	0xDF811B

//**[DF811C] max life time of torn recover log
#define TAG_DF811C_TORNLOG_MAX_LIFETIME	0xDF811C

//**[DF811D] Max Number of Torn Transaction Log Records
#define TAG_DF811D_TORNLOG_MAX_RECORDS	0xDF811D

//**[DF811E] Mag-stripe CVM Capability ï¿½?CVM Required
#define TAG_DF811E_MSD_CVMCAP_CVMREQ	0xDF811E

//**[DF811F] Security Capability
#define TAG_DF811F_SECURITY_CAP	0xDF811F

//**[DF8120] TAC-Default
#define TAG_DF8120_TAC_DEFAULT	0xDF8120

//**[DF8121] TAC-Denial
#define TAG_DF8121_TAC_DENIAL	0xDF8121

//**[DF8122] TAC-Online
#define TAG_DF8122_TAC_ONLINE	0xDF8122

//**[DF8123] Reader Contactless Floor Limit
#define TAG_DF8123_READER_CTL_FLOOR_LIMIT	0xDF8123

//**[DF8124] Reader Contactless Transaction Limit (No On-device CVM)
#define TAG_DF8124_READER_CTL_TRX_FLOOR_LIMIT_NOCDCVM	0xDF8124

//**[DF8125] Reader Contactless Transaction Limit (On-device CVM)
#define TAG_DF8125_READER_CTL_TRX_FLOOR_LIMIT_CDCVM	0xDF8125

//**[DF8126] Reader CVM Required Limit
#define TAG_DF8126_READER_CVM_LIMIT	0xDF8126

//**[DF8127] Time Out Value
#define TAG_DF8127_TIMEOUT_VALUE 0xDF8127

//**[DF8128] IDS Status
#define TAG_DF8128_IDS_STATUS	0xDF8128

//**[DF8129] Outcome Parameter Set
#define TAG_DF8129_OUTCOME_PARAM_SET 0xDF8129

//**[DF812A] DD Card (Track1)
#define TAG_DF812A_DDCARD_TRACK1	0xDF812A

//**[DF812B] DD Card (Track2)
#define TAG_DF812B_DDCARD_TRACK2	0xDF812B

//**[DF812C] Mag-stripe CVM Capability ï¿½?No CVM Required
#define TAG_DF812C_MSD_CVMCAP_NOCVM	0xDF812C

//**[DF812D] Message Hold Time
#define TAG_DF812D_MESSAGE_HOLD_TIME	0xDF812D

//**[DF8130]Hold Time Value
#define TAG_DF8130_HOLD_TIME_VALUE	0xDF8130

//**[DF8131]Phone Message Table
#define TAG_DF8131_PHONE_MESS_TABLE	0xDF8131

//**[DF8132]Minimum Relay Resistance Grace Period
#define TAG_DF8132_MIN_RR_GRACE_PERIOD	0xDF8132

//**[DF8133]Maximum Relay Resistance Grace Period
#define TAG_DF8133_MAX_RR_GRACE_PERIOD	0xDF8133

//**[DF8134]Terminal Expected Transmission Time For Relay Resistance C-APDU
#define TAG_DF8134_TERMEXPECTTRANSTIME_FOR_RRP_CAPDU	0xDF8134

//**[DF8135]Terminal Expected Transmission Time For Relay Resistance R-APDU
#define TAG_DF8135_TERMEXPECTTRANSTIME_FOR_RRP_RAPDU	0xDF8135

//**[DF8136]Relay Resistance Accuracy Threshold
#define TAG_DF8136_RR_ACCURACY_THRESHOLD	0xDF8136

//**[DF8137]Relay Resistance Transmission Time Mismatch Threshold
#define TAG_DF8137_RR_TRANSTIME_MISMATCH_THRESHOLD	0xDF8137

//**[DF8301]Terminal Relay Resistance Entropy
#define TAG_DF8301_TERM_RR_ENTROPY	0xDF8301

//**[DF8302]Device Relay Resistance Entropy
#define TAG_DF8302_DEVICE_RR_ENTROPY	0xDF8302

//**[DF8303]Min Time For Processing Relay Resistance APDU
#define TAG_DF8303_MIN_TIME_FOR_PROCESS_RR_APDU	0xDF8303

//**[DF8304]Max Time For Processing Relay Resistance APDU
#define TAG_DF8304_MAX_TIME_FOR_PROCESS_RR_APDU	0xDF8304

//**[DF8305]Device Estimated Transmission Time For Relay Resistance R-APDU
#define TAG_DF8305_DEVICE_EST_TRANSTIME_RR_RAPDU	0xDF8305

//**[DF8306]Measured Relay Resistance Processing Time
#define TAG_DF8306_MEASURED_RR_PROCESS_TIME	0xDF8306

//**[DF8307]RRP Counter
#define TAG_DF8307_RRP_COUNTER	0xDF8307

//**[FF8102]Tags To Write Before Gen AC
#define TAG_FF8102_TAGS_TO_WRITE_BEFOREGAC	0xFF8102

//**[FF8103]Tags To Write After Gen AC
#define TAG_FF8103_TAGS_TO_WRITE_AFTERGAC	0xFF8103

//**[FF8104]Data To Send
#define TAG_FF8104_DATA_TO_SEND	0xFF8104

//**[FF8105]Data Record
#define TAG_FF8105_DATA_RECORD	0xFF8105

#define TAG_FF8106_DISCRETIONARY_DATA	0xFF8106

#define TAG_FF8112_TAGS_TO_READ_YET		0xFF8112

#define TAG_FF8113_TAGS_TO_WRITE_YET_AFTERGAC	0xFF8113

#define TAG_FF8114_TAGS_TO_WRITE_YET_BEFOREGAC	0xFF8114

#define TAG_FF8115_TAGS_MISSING_PDOLDATA_FLAG	0xFF8115



/**
 * Self-defined tags for internal usage.
 * These tag names are just defined according to
 * current hash value spread. So if the hash
 * function changes in emvbuf.c leading to
 * a different spread, these tag names may
 * be changed too.
 * Always use these macros instead of
 * any real integer.
 */
#define TAG_PDOL_VALUE          0XB6
#define TAG_CDOL1_VALUE         0xB7
#define TAG_CDOL2_VALUE         0xB8
#define TAG_AUTH_DATA           0xB9
#define TAG_ISSUER_PK           0xBA
#define TAG_ICCARD_PK           0xBB
#define TAG_ONLINE_PIN          0xBD

/** Cumulative Total Transaction Amount | ÀÛ¼ÆÍÑ»ú½»Ò×½ð¶î */
#define TAG_CUMULATIVE_TOTAL_TRANSACTION_AMOUNT   0XC5
#define TAG_CTTA   TAG_CUMULATIVE_TOTAL_TRANSACTION_AMOUNT

/** Cumulative Total Transaction Amount Limit | ÀÛ¼ÆÍÑ»ú½»Ò×½ð¶îÏÞÖÆÊý */
#define TAG_CUMULATIVE_TOTAL_TRANSACTION_AMOUNT_LIMI   0XC6
#define TAG_CTTAL   TAG_CUMULATIVE_TOTAL_TRANSACTION_AMOUNT_LIMI

#define TAG_ISSUER_SCRIPT_71_BASE   0XDF8100
#define TAG_ISSUER_SCRIPT_72_BASE   0XDF8200



#define TAG_TRANSACTION_REFERENCE_CURRENCY_CONVERSION   0XDF8307





/** Í¨ÖªÖ¸Ê¾Æ÷ */
#define TAG_ADVICE_INDICATOR   0XDF8405

/** Ç©ÃûÖ¸Ê¾Æ÷ */
#define TAG_SIGNATURE_INDICATOR   0XDF8406


/** Ò»ÖÂÐÔÉùÃ÷ */
#define TAG_ICS   0X1F01
/** ×´Ì¬¼ì²éÖ§³ÖÖ¸Ê¾Æ÷ */
#define TAG_STATUS_CHECK_SUPPORT_INDICATOR   0X1F02
/** Ä¬ÈÏTDOL */
#define TAG_DEFAULT_TDOL   0X1F03
#define TAG_EXTENDED_SELECT_SUPPORT	0x1F30


//////////////////////////////////////////////////////
//0XDF8F00 ~ 0XDF8F7F : ·ÇÓ¦ÓÃÊý¾Ý£¬ÄÚºË×¨ÓÃ
//////////////////////////////////////////////////////

/** ÄÚºË×¨ÓÃÊý¾Ý»ùÖµ£¨·ÇÓ¦ÓÃÊý¾Ý£¬ÄÚºË×¨ÓÃ£© */
#define TAG_KERNEL_BASE   0XDF8800

/** ÄÚºË°æ±¾ºÅ£¨·ÇÓ¦ÓÃÊý¾Ý£¬ÄÚºË×¨ÓÃ£© */
#define TAG_KERNEL_VERSION   0XDF8F01

/** ÄÚºËChkSum£¨·ÇÓ¦ÓÃÊý¾Ý£¬ÄÚºË×¨ÓÃ£© */
#define TAG_KERNEL_CHECKSUM   0XDF8F02

/** ÅäÖÃChkSum£¬0±íÊ¾Ä¬ÈÏÅäÖÃ£¨·ÇÓ¦ÓÃÊý¾Ý£¬ÄÚºË×¨ÓÃ£© */
#define TAG_CONFIG_CHECKSUM   0xDF8F03

/** Ó¦ÓÃºòÑ¡ÁÐ±í£¨·ÇÓ¦ÓÃÊý¾Ý£¬ÄÚºË×¨ÓÃ£© */
#define TAG_CANDIDATE_LIST   0XDF8F04

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////


#endif /* EMV_TAG_H_ */
/*
************************************************************************************************************************
*                                        COPYRIGHT (C) 2014 Kingsee Tech. Co., Ltd.               ~~~ END OF FILE ~~~
************************************************************************************************************************
*/
