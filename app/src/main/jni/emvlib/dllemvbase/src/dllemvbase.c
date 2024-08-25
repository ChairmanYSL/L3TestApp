#include "dllemvbase.h"
#include "dllemvbasedebug.h"
#include <stdarg.h>
#include <stdio.h>

EMVBase_LogOutput *gstemvbaselogoutput = NULL;

void EMVBase_SetLogOutput(EMVBase_LogOutput *pstLogOutput)
{
	gstemvbaselogoutput = NULL;
	if(pstLogOutput && pstLogOutput->EmvBase_Printf && pstLogOutput->logoutputswitch)
	{
		gstemvbaselogoutput = pstLogOutput;
	}
}

void EMVBase_Trace(char *fmt, ...)
{
#ifdef EMVBASE_DEBUG
	va_list args;
	unsigned char printbuffer[256] = {0};

	if(gstemvbaselogoutput && gstemvbaselogoutput->EmvBase_Printf && gstemvbaselogoutput->logoutputswitch)
	{
		va_start ( args, fmt );
		vsprintf ( printbuffer, fmt, args );
		va_end ( args );
		gstemvbaselogoutput->EmvBase_Printf("%s",printbuffer);
	}
#endif
}

void  EMVBase_TraceHex(unsigned char const* pasInfo, void const *pheData, int siLen)
{
#ifdef EMVBASE_DEBUG

    int i;
    unsigned char temp[200] = {0};
    unsigned char buf[16] = {0};
    unsigned char const* src  = (unsigned char const*)pheData;

	if(pasInfo)
	{
	    strcpy(temp, pasInfo);
	    EMVBase_Trace("%s:",temp);
	    memset(temp, 0, sizeof(temp));
	}

    for(i = 0; i < siLen; i++)
    {
        if((i != 0) && (i % 16 == 0))
        {
            EMVBase_Trace("%s\r\n",temp);
            memset(temp, 0, sizeof(temp));
        }
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%02X ", src[i]);
        strcat(temp, buf);
    }
    EMVBase_Trace("%s\r\n",temp);
#endif
}


static int GetCompileTime(char *pasCompileTime, const char *pasDate, const char *pasTime)
{
	char temp_date[64] = {0},str_year[5] = {0}, str_month[4] = {0}, str_day[3] = {0};
	char temp_time[64] = {0},str_hour[2] = {0}, str_min[2] = {0}, str_sec[2] = {0};
	char en_month[12][4]={ "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	char tempCompileTime[20] = {0};
	int i = 0;

	if(pasCompileTime == NULL || pasDate == NULL || pasTime == NULL)
	{
		return 0;
	}

	sprintf(temp_date,"%s",pasDate);    //"Sep 7 2012"
	sscanf(temp_date,"%s %s %s",str_month, str_day, str_year);

	for(i=0; i < 12; i++)
	{
		if(strncmp(str_month,en_month[i],3)==0)
		{
		    memset(str_month, 0, sizeof(str_month));
	  		sprintf(str_month, "%02d", i + 1);
	        break;
	    }
	}

	if(strlen(str_day)==1)//锟斤拷锟斤拷锟斤拷为1位锟斤拷要前锟斤拷0x30
	{
		str_day[1]=str_day[0];
		str_day[0]=0x30;
	}

	sprintf(pasCompileTime, "%s%s%s", str_year, str_month, str_day);
	sprintf(tempCompileTime, " %s", pasTime);
	strcat(pasCompileTime, tempCompileTime);

	return 0;
}

void EMVBase_GetLibVersion(unsigned char *version)
{
	GetCompileTime(version, __DATE__, __TIME__);//如2020年01月21日编译，输出version为:200121
}


unsigned char EMVBase_ParseLen(unsigned char *ParseData,unsigned short index,unsigned short *DataLen)
{
     unsigned char t;
     unsigned short j,k,templen;


	k = index;
	if(ParseData[k]<=127)
	{
		templen=ParseData[k];
		k++;
	}
	else
	{
		templen=0;
		t=ParseData[k]&0x7F;
		for(j=1;j<=t;j++)
		{
		    if((k+j)>299)
		    {
		    	return 1;
		    }
			templen=templen*256+ParseData[k+j];
		}
		k+=t+1;
	}
	*DataLen = templen;
	return 0;
}

unsigned char EMVBase_ParseExtLen(unsigned char *ParseData,unsigned short *index,unsigned short *DataLen)
{
     unsigned char t;
     unsigned short j,k,templen;


	k = *index;
	if(ParseData[k]<=127)
	{
		templen=ParseData[k];
		k++;
	}
	else
	{
		templen=0;
		t=ParseData[k]&0x7F;
		if (t>2)
		{
	    	return 1;
		}
		for(j=1;j<=t;j++)
		{
			if((k+j)>299)
		    {
		    	return 1;
		    }
			templen=templen*256+ParseData[k+j];
		}
		k+=t+1;
	}
	*index = k;
	*DataLen = templen;
	return 0;
}


unsigned char EMVBase_ParseExtLenWithMaxLen(unsigned char *ParseData,unsigned short *index,unsigned short *DataLen,unsigned short MaxLen)
{
     unsigned char t;
     unsigned short j,k,templen;


	k = *index;
	if(ParseData[k]<=127)
	{
		templen=ParseData[k];
		k++;
	}
	else
	{
		templen=0;
		t=ParseData[k]&0x7F;
		if (t>2)
		{
	    	return 1;
		}
		for(j=1;j<=t;j++)
		{
			if((k+j)>MaxLen)
		    {
		    	return 1;
		    }
			templen=templen*256+ParseData[k+j];
		}
		k+=t+1;
	}
	*index = k;
	*DataLen = templen;
	return 0;
}


unsigned char EMVBase_ReadSelectRetData(EMVBASE_SELECT_RET* selectRet, unsigned char* DataOut, unsigned short LenOut, EMVBase_UnionStruct *tempAppUnionStruct)
{
    int matchnum;
    EMVBASE_LISTAPPDATA *tempAppAppData;
    unsigned char *tempappnum;
    unsigned char j, k;
    unsigned short index, indexFCI, indexFCIProp, len, lenFCI, lenFCIProp;
    int indexIssuerDiscret, lenIssuerDiscret;
    int indexAppApp, lenAppApp;
    EMVBASE_TERMAPP tempAppTermApp;
    unsigned short templen;
	unsigned char extendedselect[11] = {0};
	unsigned short extendselectlen = 0;
	unsigned short indexttemp=0,tagLENLen=0,tagLen=0;
	int rfulen=0;

    tempappnum = &(tempAppUnionStruct->EntryPoint->AppListCandidatenum);
    tempAppAppData = tempAppUnionStruct->EntryPoint->AppListCandidate;
    *tempappnum = 0;


    index = 0;

    if(DataOut[index] != 0x6F)
    {
        return RLT_ERR_EMV_IccDataFormat;
    }
    index++;

    if(EMVBase_ParseExtLen(DataOut, &index, &lenFCI))
    {
        return RLT_ERR_EMV_IccDataFormat;
    }
    indexFCI = index;

    while(index < indexFCI + lenFCI)
    {
        if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

        if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
        else if(DataOut[index] == 0x84)
        {
            if(selectRet->DFNameExist == 1) { return RLT_ERR_EMV_IccDataFormat; }
            index++;

			if(EMVBase_ParseExtLen(DataOut, &index, &len))
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
            selectRet->DFNameLen = len;
			if(tempAppUnionStruct->CheckTag84 & 0x02)
			{
				if(selectRet->DFNameLen != 14) { return RLT_ERR_EMV_IccDataFormat; }
			}

            if(selectRet->DFNameLen > 16) { return RLT_ERR_EMV_IccDataFormat; }

            memcpy(selectRet->DFName, DataOut + index ,selectRet->DFNameLen);
			if(tempAppUnionStruct->CheckTag84 & 0x04)
			{
				if(memcmp(selectRet->DFName, "2PAY.SYS.DDF01", 14)) { return RLT_ERR_EMV_IccDataFormat; }
			}
            index += selectRet->DFNameLen;
            selectRet->DFNameExist = 1;
        }
        else if(DataOut[index] == 0xA5)
        {
            selectRet->FCIPropExist = 1;
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &lenFCIProp))
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
            indexFCIProp = index;


            while(index < indexFCIProp + lenFCIProp)
            {
                if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

                if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
                else if(!memcmp(DataOut + index, "\xBF\x0C", 2))
                {
                    if(selectRet->IssuerDiscretExist == 1) { return RLT_ERR_EMV_IccDataFormat; }
                    index += 2;

                    if(EMVBase_ParseExtLen(DataOut, &index, &len))
                    {
                        return RLT_ERR_EMV_IccDataFormat;
                    }

                    if(len > 253) { return RLT_ERR_EMV_IccDataFormat; }
                    selectRet->IssuerDiscretLen = len;
                    selectRet->IssuerDiscretExist = 1;

                    indexIssuerDiscret = index;
                    lenIssuerDiscret = len;

                    while(index < indexIssuerDiscret + lenIssuerDiscret)
                    {
                        if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

                        if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
                        else if(DataOut[index] == 0x61)
                        {
							if((*tempappnum) >= tempAppUnionStruct->EntryPoint->AppListCandidateMaxNum)//���������ø���,���ٸ�ֵ
							{
								k = DataOut[index];
	                            index++;

	                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
	                            {
	                                return RLT_ERR_EMV_IccDataFormat;
	                            }
	                            index += len;
								continue;
							}


                        	selectRet->ApplicationTemplateExist = 1;
                            memset(tempAppAppData + (*tempappnum), 0, sizeof(EMVBASE_LISTAPPDATA));
                            index += 1;

                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
                            {
                                return RLT_ERR_EMV_IccDataFormat;
                            }
                            indexAppApp = index;
                            lenAppApp = len;

                            while(index < indexAppApp + lenAppApp)
                            {
                                if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

                                if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
                                else if(DataOut[index] == 0x4F)
                                {
                                    index += 1;

                                    if(EMVBase_ParseExtLen(DataOut, &index, &templen))
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }
                                    (tempAppAppData + (*tempappnum))->AIDLen = templen;

                                    if((tempAppAppData + (*tempappnum))->AIDLen > 16) { (tempAppAppData + (*tempappnum))->AIDLen = 0; }

                                    j = (tempAppAppData + (*tempappnum))->AIDLen;
                                    memcpy((tempAppAppData + (*tempappnum))->AID, &DataOut[index], j);
                                    index += templen;
                                }
                                else if(DataOut[index] == 0x50)		//AppLabel
                                {
                                    index += 1;

                                    if(EMVBase_ParseExtLen(DataOut, &index, &templen))
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }
                                    (tempAppAppData + (*tempappnum))->AppLabelLen = templen;

                                    if((tempAppAppData + (*tempappnum))->AppLabelLen > 16) { (tempAppAppData + (*tempappnum))->AppLabelLen = 16; }

                                    j = (tempAppAppData + (*tempappnum))->AppLabelLen;
                                    memcpy((tempAppAppData + (*tempappnum))->AppLabel, DataOut + index, j);
                                    index += templen;
                                }
                                else if(DataOut[index] == 0x87)		//AppPriority
                                {
                                    index += 1;

                                    if(EMVBase_ParseExtLen(DataOut, &index, &templen))
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }

                                    if(templen != 1)
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }
                                    (tempAppAppData + (*tempappnum))->Priority = DataOut[index];
                                    index++;
                                }
								else if(!memcmp(DataOut+index,"\x9F\x2A",2))  //Kernel Identifier
								{
									index+=2;

									if(EMVBase_ParseExtLen(DataOut,&index,&templen))
									{
										return RLT_ERR_EMV_IccDataFormat;
									}

									(tempAppAppData + (*tempappnum))->KernelIdentifierLen  = templen;

									if((tempAppAppData + (*tempappnum))->KernelIdentifierLen > 8)
									{
										(tempAppAppData + (*tempappnum))->KernelIdentifierLen = 8;
									}
									memcpy((tempAppAppData + (*tempappnum))->KernelIdentifier,DataOut+index,(tempAppAppData + (*tempappnum))->KernelIdentifierLen);
									index+=templen;
								}
								else if(!memcmp(DataOut + index, "\x9F\x29", 2))
                                {
                                    index += 2;

                                    if(EMVBase_ParseExtLen(DataOut, &index, &templen))
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }
                                    extendselectlen = templen;

                                    if(extendselectlen <= 11)
                                    {
                                        memcpy(extendedselect, DataOut + index, extendselectlen);
                                    }
                                    else
                                    {
                                        extendselectlen = 0;
                                    }
                                    index += templen;
                                }
								else if(!memcmp(DataOut + index, "\x9F\x0A", 2))
								{
									index+=2;

									if(EMVBase_ParseExtLen(DataOut,&index,&templen))
									{
										return RLT_ERR_EMV_IccDataFormat;
									}

									(tempAppAppData + (*tempappnum))->ASRPDLen  = templen;

									if(templen > 0)
									{
										(tempAppAppData + (*tempappnum))->ASRPD = emvbase_malloc(templen);
										memcpy((tempAppAppData + (*tempappnum))->ASRPD,DataOut+index,templen);

									}
									index+=templen;
								}
                                else if(DataOut[index] == 0x61)
                                {
                                    break;
                                }
                                else
                                {
                                	tagLen=0;
									indexttemp = index;
                                    k = DataOut[index];

                                    if((k & 0x1F) == 0x1F)
                                    {
                                        index++;
										tagLen++;
                                    }
                                    index++;
									tagLen++;

									tagLENLen = index;
                                    if(EMVBase_ParseExtLen(DataOut, &index, &len))
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }
									tagLENLen = index-tagLENLen;
									tagLen = (len+tagLENLen+tagLen);
									if(tagLen > 0)
									{
										if (NULL != (tempAppAppData + (*tempappnum))->rfu)
										{
											unsigned char *temp = (tempAppAppData + (*tempappnum))->rfu;

											(tempAppAppData + (*tempappnum))->rfu = emvbase_malloc((tempAppAppData + (*tempappnum))->rfuLen + tagLen);
											if((tempAppAppData + (*tempappnum))->rfu == NULL)
											{
												return RLT_EMV_ERR;
											}
											memcpy((tempAppAppData + (*tempappnum))->rfu,temp,(tempAppAppData + (*tempappnum))->rfuLen);
											emvbase_free(temp);
											temp=NULL;
											memcpy((tempAppAppData + (*tempappnum))->rfu + (tempAppAppData + (*tempappnum))->rfuLen, DataOut+indexttemp, tagLen);
											(tempAppAppData + (*tempappnum))->rfuLen  += tagLen;

										}
										else
										{
											(tempAppAppData + (*tempappnum))->rfu = emvbase_malloc(tagLen);
											if((tempAppAppData + (*tempappnum))->rfu == NULL)
											{
												return RLT_EMV_ERR;
											}
											memcpy((tempAppAppData + (*tempappnum))->rfu,DataOut+indexttemp,tagLen);
											(tempAppAppData + (*tempappnum))->rfuLen  = tagLen;
										}
									}

                                    index += len;
                                }
                            }

                            if((tempAppAppData + (*tempappnum))->AIDLen != 0 && (*tempappnum) < tempAppUnionStruct->EntryPoint->AppListCandidateMaxNum)
                            {
                                matchnum = tempAppUnionStruct->CheckMatchTermAID((tempAppAppData + (*tempappnum))->AID, (tempAppAppData + (*tempappnum))->AIDLen, &tempAppTermApp,(tempAppAppData + (*tempappnum))->KernelIdentifier, (tempAppAppData + (*tempappnum))->KernelIdentifierLen);

                                if(matchnum >= 0)
                                {
                                	if(tempAppUnionStruct->SupportExternSelect && extendselectlen && ((tempAppAppData + (*tempappnum))->AIDLen + extendselectlen) <= 16)
		                            {
		                                memcpy((tempAppAppData + (*tempappnum))->AID + (tempAppAppData + (*tempappnum))->AIDLen, extendedselect, extendselectlen);
		                                (tempAppAppData + (*tempappnum))->AIDLen += extendselectlen;
		                            }

                                    (tempAppAppData + (*tempappnum))->AidInTermLen = tempAppTermApp.AIDLen;
                                    memcpy((tempAppAppData + (*tempappnum))->AidInTerm, tempAppTermApp.AID, (tempAppAppData + (*tempappnum))->AidInTermLen);
                                    (*tempappnum)++;
                                }
                            }

							extendselectlen = 0;
							memset(extendedselect, 0, sizeof(extendedselect));
                        }
                        else if(!memcmp(DataOut + index, "\x9F\x4D", 2))	//Log Entry
                        {
                            if(selectRet->LogEntryExist == 1) { return RLT_ERR_EMV_IccDataFormat; }
                            index += 2;

                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
                            {
                                return RLT_ERR_EMV_IccDataFormat;
                            }
                            selectRet->LogEntryLen = len;
                            index += selectRet->LogEntryLen;
                            selectRet->LogEntryExist = 1;
                        }
						else if(!memcmp(DataOut + index, "\x9F\x3E", 2))	// 20210708   Terminal Categories Supported List
                        {
                            index += 2;

                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
                            {
                                return RLT_ERR_EMV_IccDataFormat;
                            }

							emvbase_avl_createsettagvalue(EMVTAG_TermCateSupportedList, DataOut + index, len);
							index += len;
                        }
						else if(!memcmp(DataOut + index, "\x9F\x3F", 2))	// 20210708   Selection Data Object List (SDOL)
                        {
                            index += 2;

                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
                            {
                                return RLT_ERR_EMV_IccDataFormat;
                            }

                            emvbase_avl_createsettagvalue(EMVTAG_SDOL, DataOut + index, len);
							index += len;
                        }
                        else
                        {
                            k = DataOut[index];

                            if((k & 0x1F) == 0x1F)
                            {
                                index++;
                            }
                            index++;

                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
                            {
                                return RLT_ERR_EMV_IccDataFormat;
                            }
                            index += len;
                        }
                    }
                }
                else		//other unknown TLV data
                {
                    k = DataOut[index];

                    if((k & 0x1F) == 0x1F)
                    {
                        index++;
                    }
                    index++;

                    if(EMVBase_ParseExtLen(DataOut, &index, &len))
                    {
                        return RLT_ERR_EMV_IccDataFormat;
                    }
                    index += len;
                }
            }

            if(index != indexFCIProp + lenFCIProp)
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
        }
        else
        {
            k = DataOut[index];

            if((k & 0x1F) == 0x1F)
            {
                index++;
            }
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &len))
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
            index += len;
        }
    }

    if(index != indexFCI + lenFCI)
    {
        return RLT_ERR_EMV_IccDataFormat;
    }
    return RLT_EMV_OK;
}

unsigned char EMVBase_ReadSelectRetData_Jcb(EMVBASE_SELECT_RET* selectRet, unsigned char* DataOut, unsigned short LenOut, EMVBase_UnionStruct *tempAppUnionStruct)
{
	  int matchnum;
    EMVBASE_LISTAPPDATA *tempAppAppData;
    EMVBASE_TERMAPP tempAppTermApp;
    unsigned char *tempappnum;
    //unsigned char *temptermlistnum;
    unsigned char j, k;     //i,
    unsigned short index, indexFCI, indexFCIProp, len, lenFCI, lenFCIProp;
    int indexIssuerDiscret, lenIssuerDiscret;
    int indexAppApp, lenAppApp;
    unsigned char priorityexist;
	unsigned short templen, ExtendedLen;
	unsigned char tempindex;
	unsigned char errindex = 0,i;
	unsigned char errindex2 = 0;
	unsigned short tempBF0Clen;
	unsigned char extendedselect[11] = {0};
	unsigned short extendselectlen = 0;


    tempappnum = &(tempAppUnionStruct->EntryPoint->AppListCandidatenum);
    tempAppAppData = tempAppUnionStruct->EntryPoint->AppListCandidate;
    *tempappnum = 0;


    index = 0;

    if(DataOut[index] != 0x6F)          //FCI?���?������?
    {
        return RLT_ERR_EMV_IccDataFormat;         //FCI template
    }
    index++;

    if(EMVBase_ParseExtLen(DataOut, &index, &lenFCI))
    {
        return RLT_ERR_EMV_IccDataFormat;
    }
    indexFCI = index;

    while(index < indexFCI + lenFCI)
    {
        if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

        if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
        else if(DataOut[index] == 0x84)         //DF??
        {
            if(selectRet->DFNameExist == 1) { return RLT_ERR_EMV_IccDataFormat; }
            index++;

			if(EMVBase_ParseExtLen(DataOut, &index, &len))
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
            selectRet->DFNameLen = len;
			if(tempAppUnionStruct->CheckTag84 & 0x02)
			{
				if(selectRet->DFNameLen != 14) { return RLT_ERR_EMV_IccDataFormat; }
			}

			if(selectRet->DFNameLen > 16) { return RLT_ERR_EMV_IccDataFormat; }

			memcpy(selectRet->DFName, DataOut + index ,selectRet->DFNameLen);
			if(tempAppUnionStruct->CheckTag84 & 0x04)
			{
				if(memcmp(selectRet->DFName, "2PAY.SYS.DDF01", 14)) { return RLT_ERR_EMV_IccDataFormat; }
			}
            index += selectRet->DFNameLen;
            selectRet->DFNameExist = 1;
        }
        else if(DataOut[index] == 0xA5)         //FCI��y?Y������??���?
        {
            selectRet->FCIPropExist = 1;
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &lenFCIProp))
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
            indexFCIProp = index;

			EMVBase_Trace( "indexFCIProp=%d lenFCIProp=%d\r\n",indexFCIProp, lenFCIProp);
            while(index < indexFCIProp + lenFCIProp)
            {
                if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

                if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
                else if(!memcmp(DataOut + index, "\xBF\x0C", 2))             //FCI ����?����?��??����?��y?Y
                {
                    if(selectRet->IssuerDiscretExist == 1) { return RLT_ERR_EMV_IccDataFormat; }
                    index += 2;

                    if(EMVBase_ParseExtLen(DataOut, &index, &len))
                    {
                    	EMVBase_Trace( "bf0c error!\r\n");
                        return RLT_ERR_EMV_IccDataFormat;
                    }

                    if(len > 253) { return RLT_ERR_EMV_IccDataFormat; }
                    selectRet->IssuerDiscretLen = len;                     //sxl 090211
                    EMVBase_Trace( "IssuerDiscretLen=%d index=%d\r\n", len, index);
                    //memcpy(selectRet->IssuerDiscret,DataOut+index,len);
                    selectRet->IssuerDiscretExist = 1;

                    indexIssuerDiscret = index;
                    lenIssuerDiscret = len;

                    while(index < indexIssuerDiscret + lenIssuerDiscret)
                    {
                    	EMVBase_Trace( "index=%d indexIssuerDiscret=%d lenIssuerDiscret=%d",index, indexIssuerDiscret,lenIssuerDiscret);
                        if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

                        if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
                        else if(DataOut[index] == 0x61)
                        {
                        	selectRet->ApplicationTemplateExist = 1;
                            memset((unsigned char *)&tempAppAppData[*tempappnum], 0, sizeof(EMVBASE_LISTAPPDATA));                           //??��|��?
                            priorityexist = 0;
                            index += 1;

                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
                            {
                            	EMVBase_Trace( "tag 61 error\r\n");
                                return RLT_ERR_EMV_IccDataFormat;
                            }
                            indexAppApp = index;
                            lenAppApp = len;

							errindex = index;
							if(len == 0)
							{
								EMVBase_Trace("tag 61 length zero!\r\n");
								return RLT_ERR_EMV_IccDataFormat;
							}

                            while(index < indexAppApp + lenAppApp)
                            {
                            	EMVBase_Trace( "index=%d LenOut=%d\r\n", index, LenOut);
                                if(index >= LenOut)
								{
									EMVBase_Trace( "error11111");
									//return ERR_EMV_IccDataFormat;
									break;
								}

                                if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
                                else if(DataOut[index] == 0x4F)                                 //AID
                                {
                                    index += 1;
                                    (tempAppAppData + (*tempappnum))->AIDLen = DataOut[index];
									if((tempAppAppData + (*tempappnum))->AIDLen > 16)
									{
										(tempAppAppData + (*tempappnum))->AIDLen = 0;
									}



									index++;
                                    j = (tempAppAppData + (*tempappnum))->AIDLen;
                                    memcpy((tempAppAppData + (*tempappnum))->AID, DataOut + index, j);
                                    EMVBase_TraceHex( "4F aid &len", DataOut + index, j);
                                    index += j;
                                }
                                else if(DataOut[index] == 0x50)                                 //AppLabel
                                {//500B 4A434220437265646974870101
                                	EMVBase_Trace("goto deal tag 50!\r\n");
                                    index += 1;
                                    (tempAppAppData + (*tempappnum))->AppLabelLen = DataOut[index];

                                    if((tempAppAppData + (*tempappnum))->AppLabelLen > 16) { (tempAppAppData + (*tempappnum))->AppLabelLen = 16; }
                                    index++;
                                    j = (tempAppAppData + (*tempappnum))->AppLabelLen;
                                    memcpy((tempAppAppData + (*tempappnum))->AppLabel, DataOut + index, j);
                                    index += j;
                                }
                                else if(DataOut[index] == 0x87)    //AppPriority
                                {
									EMVBase_Trace("goto deal tag 87!\r\n");
									index ++;
									templen = DataOut[index];
									if(templen == 1)//870201   870001	87020001   870101
									{
										index++;
										(tempAppAppData + (*tempappnum))->Priority = DataOut[index];
										index++;
										priorityexist = 1;

									}
									else
									{
										index++;
										templen = 0;
										index +=templen;
									}

                                }
								else if(memcmp(DataOut + index, "\x9F\x2A", 2) == 0)//kernel ID
								{//9F2A0105  9F2A04050001  9F2A0305000001  9F2A0005

									index += 2;
									templen = DataOut[index];
									EMVBase_Trace( "tag9F2A templen=%d\r\n", templen);
									if(templen == 1)
									{
										index++;
										EMVBase_Trace( "tag9F2A 1byte=%02x\r\n", DataOut[index]);
										if(DataOut[index] != 0x05)
										{
											templen = 0;
											(tempAppAppData + (*tempappnum))->AIDLen = 0;
											//index--;
										}
										index += templen;
									}
									else
									{
										index++;
										templen = 0;
										index += templen;
									}
								}
								else if(memcmp(DataOut + index, "\x9F\x29", 2) == 0)//Extened Selection zwb 180202
								{  //9F29021122  9F29031122  9F29011122  9F29001122
									EMVBase_Trace( "goto deal tag 9F29!\r\n");
								#if 0
									index += 2;
									templen = DataOut[index];
									if(templen == 2)
									{
										index++;
										index += templen;
									}
									else
									{
										index++;
										templen =  0;
										index += templen;
									}
								#else //sjz 2020.6.19
									index += 2;

									if(EMVBase_ParseExtLen(DataOut, &index, &templen))
									{
										return RLT_ERR_EMV_IccDataFormat;
									}
									if(templen <= 2)
									{
										extendselectlen = templen;

										if(extendselectlen <= 11)
										{
											memcpy(extendedselect, DataOut + index, extendselectlen);
										}
										else
										{
											extendselectlen = 0;
										}
										index += templen;
									}

								#endif
								}
                                else
                                {
									errindex = index;

                                    k = DataOut[index];
									EMVBase_Trace( "xxx k=%02x\r\n", k);

                                    if((k & 0x1F) == 0x1F)
                                    {
                                        index++;
                                    }
                                    index++;


									EMVBase_Trace( "index111=%d indexAppApp + lenAppApp=%d\r\n", index, indexAppApp + lenAppApp);
									EMVBase_Trace( "DataOut[index]=%02x\r\n", DataOut[index]);
									if(index < indexAppApp + lenAppApp)
									{
										if(EMVBase_ParseExtLen(DataOut, &index, &len))
										{
											break;
										}
										EMVBase_Trace( "test111111");
									}
									else
									{
										break;
									}

                                    index += len;
                                }

                            }

							EMVBase_Trace("00000index=%d LenOut=%d",index,LenOut);

							EMVBase_Trace("00000index=%d indexAppApp + lenAppApp=%d\r\n", index, indexAppApp + lenAppApp);
							if(index >  indexAppApp + lenAppApp /*&& index < LenOut*/)	// parase err
							{
								EMVBase_Trace("errindex=%d\r\n", errindex);
								index = errindex;
								for(i = errindex ; i < indexAppApp + lenAppApp ;  i++)
								{
									if(DataOut[i] == 0x61)
									{
										EMVBase_Trace("another tag 61");
										break;
									}
								}
								index = i;

							}

                            if((tempAppAppData + (*tempappnum))->AIDLen != 0 && (*tempappnum) < tempAppUnionStruct->EntryPoint->AppListCandidateMaxNum)
                            {
                                matchnum = tempAppUnionStruct->CheckMatchTermAID((tempAppAppData + (*tempappnum))->AID, (tempAppAppData + (*tempappnum))->AIDLen, &tempAppTermApp,(tempAppAppData + (*tempappnum))->KernelIdentifier, (tempAppAppData + (*tempappnum))->KernelIdentifierLen);
								EMVBase_Trace("matchnum = %d\r\n",matchnum);

                                if(matchnum >= 0)
                                {
                                	if(tempAppUnionStruct->SupportExternSelect && extendselectlen && ((tempAppAppData + (*tempappnum))->AIDLen + extendselectlen) <= 16)
		                            {//sjz 2020.6.19
		                                memcpy((tempAppAppData + (*tempappnum))->AID + (tempAppAppData + (*tempappnum))->AIDLen, extendedselect, extendselectlen);
		                                (tempAppAppData + (*tempappnum))->AIDLen += extendselectlen;
		                            }

                                    (tempAppAppData + (*tempappnum))->AidInTermLen = tempAppTermApp.AIDLen;
                                    memcpy((tempAppAppData + (*tempappnum))->AidInTerm, tempAppTermApp.AID, (tempAppAppData + (*tempappnum))->AidInTermLen);
                                    (*tempappnum)++;

									EMVBase_Trace("tempappnum = %d\r\n",(*tempappnum));
                                }
                            }

							extendselectlen = 0;
							memset(extendedselect, 0, sizeof(extendedselect));
                        }
                        else if(!memcmp(DataOut + index, "\x9F\x4D", 2))                    //Log Entry��???��??��
                        {
                            if(selectRet->LogEntryExist == 1) { return RLT_ERR_EMV_IccDataFormat; }
                            index += 2;

                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
                            {
                                return RLT_ERR_EMV_IccDataFormat;
                            }
                            selectRet->LogEntryLen = len;
                            //memcpy(selectRet->LogEntry,DataOut+index,selectRet->LogEntryLen);
                            index += selectRet->LogEntryLen;
                            selectRet->LogEntryExist = 1;
                        }
                        else
                        {
                            if(errindex2 == 0)
                            {
                            	errindex2 = index;
                            }
                            k = DataOut[index];
							EMVBase_Trace("errindex2 k=%02x\r\n", k);

                            if((k & 0x1F) == 0x1F)
                            {
                                index++;
                            }
                            index++;

							EMVBase_Trace("index2222=%d indexIssuerDiscret + lenIssuerDiscret=%d\r\n", index, indexIssuerDiscret + lenIssuerDiscret);

							if(index < indexIssuerDiscret + lenIssuerDiscret)
							{
								EMVBase_Trace("DataOut[index]=%02x\r\n", DataOut[index]);
								if(EMVBase_ParseExtLen(DataOut, &index, &len))
								{
									break;
								}
							}
							else
							{
								break;
							}

                            index += len;
                        }
                    }

					EMVBase_Trace("index3333=%d indexIssuerDiscret + lenIssuerDiscret=%d\r\n", index, indexIssuerDiscret + lenIssuerDiscret);
					if(index >	indexIssuerDiscret + lenIssuerDiscret)	// parase err
					{
						EMVBase_Trace("errindex2=%d\r\n", errindex2);
						index = errindex2;
						for(i = errindex2 ; i < indexIssuerDiscret + lenIssuerDiscret ; i++)
						{
							if(DataOut[i] == 0x61)
							{
								break;
							}
						}
						index = i;

					}
				}
                else                //other unknown TLV data
                {
                    k = DataOut[index];
					EMVBase_Trace( "2222k=%d\r\n",k);

                    if((k & 0x1F) == 0x1F)
                    {
                        index++;
                    }
                    index++;

                    if(EMVBase_ParseExtLen(DataOut, &index, &len))
                    {
						EMVBase_Trace( "yyyyyyyyyyyy\r\n");
                        return RLT_ERR_EMV_IccDataFormat;
                    }
                    index += len;
                }
            }

			EMVBase_Trace("IssuerDiscretLen=%d index=%d indexIssuerDiscret=%d\r\n", selectRet->IssuerDiscretLen, index, indexIssuerDiscret);
			if( selectRet->IssuerDiscretLen != (index - indexIssuerDiscret))// BF0C length value
			{// the length  must correspond to the length of value
				EMVBase_Trace("BF0C length value error!\r\n");
				return RLT_ERR_EMV_IccDataFormat;
			}

            if(index != indexFCIProp + lenFCIProp)
            {
				EMVBase_Trace( "A5 length value error!\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
        }
        else
        {
            k = DataOut[index];

            if((k & 0x1F) == 0x1F)
            {
                index++;
            }
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &len))
            {
				EMVBase_Trace( "aaaaaaaaaa\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
            index += len;
        }
    }

    if(index != indexFCI + lenFCI)
    {
		EMVBase_Trace( "bbbbbbbbbbb\r\n");
        return RLT_ERR_EMV_IccDataFormat;
    }
    return RLT_EMV_OK;
}


unsigned char EMVBase_SelectDFRetData(EMVBASE_APDU_RESP *apdu_r, EMVBase_UnionStruct *tempAppUnionStruct)
{
    EMVBASE_SELECT_RET selectRet;

    if(apdu_r->SW1 == 0x90 && apdu_r->SW2 == 0x00)
    {
        memset((unsigned char*)&selectRet, 0, sizeof(EMVBASE_SELECT_RET));

		if(tempAppUnionStruct->PpseRespType == 1)
		{
			if(EMVBase_ReadSelectRetData_Jcb(&selectRet, apdu_r->DataOut, apdu_r->LenOut, tempAppUnionStruct) != RLT_EMV_OK)
			{
				return RLT_ERR_EMV_IccDataFormat;
			}
		}
		else
		{
			if(EMVBase_ReadSelectRetData(&selectRet, apdu_r->DataOut, apdu_r->LenOut, tempAppUnionStruct) != RLT_EMV_OK)
			{
				EMVBase_Trace("EMVBase_SelectDFRetData-line: %d\r\n", __LINE__);
				return RLT_ERR_EMV_IccDataFormat;
			}
		}

		if(tempAppUnionStruct->CheckTag84 & 0x01)
		{
			if(selectRet.DFNameExist == 0)
			{
				EMVBase_Trace("EMVBase_SelectDFRetData-line: %d\r\n", __LINE__);
				return RLT_ERR_EMV_IccDataFormat;
			}
		}
		if(selectRet.ApplicationTemplateExist == 0)
		{
			return RLT_EMV_TERMINATE_TRANSERR;
		}

        if(selectRet.FCIPropExist == 0)
        {
        	EMVBase_Trace("EMVBase_SelectDFRetData-line: %d\r\n", __LINE__);
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(selectRet.SFIExist == 1)
        {
            if(selectRet.SFI < 1 || selectRet.SFI > 10 )
            {
            	EMVBase_Trace("EMVBase_SelectDFRetData-line: %d\r\n", __LINE__);
                return RLT_ERR_EMV_IccDataFormat;
            }
        }

        return RLT_EMV_OK;
    }
    else if(apdu_r->SW1 == 0x6A && apdu_r->SW2 == 0x81)
    {
        return RLT_ERR_EMV_CardBlock;
    }
	else if(apdu_r->SW1 == 0x6A && apdu_r->SW2 == 0x82)
	{
		return RLT_EMV_PPSE_REV_6A82;
	}
    else
    {
        return RLT_ERR_EMV_IccReturn;
    }
}

unsigned char EMVBase_SDOLProcess(EMVBase_UnionStruct *tempAppUnionStruct, EMVBASE_APDU_SEND *apdu_s, unsigned char SPICondition, unsigned char *termCategory)
{
    unsigned char *sdolData;
    unsigned short index, indexOut;
    unsigned char i, k, m, bInTable;
    unsigned char len;
    EMVBASETAGCVLITEM *item;
    unsigned char *SDOL;
    unsigned char tag[3], taglen;
    EMVBASETAGCVLITEM *sdoldataitem = NULL;
    unsigned short templen, tempSDOLlen;
    unsigned char SDOLbexist = 0;
    unsigned char *pdata, *buf;

    item = emvbase_avl_gettagitempointer(EMVTAG_SDOL);

    if(item != NULL)
    {
        if(item->len)
        {
            SDOLbexist = 1;
        }
    }

    if(SDOLbexist == 0)
    {
        memcpy(apdu_s->Command, "\x80\x1A\x00\x00", 4);

        apdu_s->Lc = 2 + 5;

		apdu_s->DataIn[0] = 0x83;
		apdu_s->DataIn[1] = 5;
		memcpy(apdu_s->DataIn+2, "\x00\x01\x02", 3);
		memcpy(apdu_s->DataIn+2+3, termCategory, 2);

        apdu_s->Le = 256;
        apdu_s->EnableCancel = 1;
        return RLT_EMV_OK;
    }

    pdata = (unsigned char *)emvbase_malloc(1024);
    sdolData = &pdata[0];
    SDOL = &pdata[300];
    buf = &pdata[700];

    memset(sdolData, 0, 255);

    index = 0;
    indexOut = 0;

    memset(SDOL, 0, 300);

    if(item->len > 255)
    {
        tempSDOLlen = 255;
    }
    else
    {
        tempSDOLlen = item->len;
    }
    memcpy(SDOL, item->data, tempSDOLlen);

    while(index < tempSDOLlen)  		//Process SDOL
    {
        if(SDOL[index] == 0xFF || SDOL[index] == 0x00) {index++; continue; }
        memset(buf, 0, 255);
        bInTable = 0;

        memset(tag, 0, sizeof(tag));
        taglen = 0;
        tag[taglen++] = SDOL[index];

        if((tag[0] & 0x1f) == 0x1f)
        {
            tag[taglen++] = SDOL[index + 1];

            if(tag[1] & 0x80)
            {
                tag[taglen++] = SDOL[index + 2];
            }
        }

		//if((memcmp(tag, "\x9F\x50", 2) < 0) || (memcmp(tag, "\x9F\x7F", 2) > 0) || (!memcmp(tag, "\x9F\x7A", 2)))
		{
        	sdoldataitem = emvbase_avl_gettagitempointer(tag);

        	if(sdoldataitem != NULL)
	        {
	            index += taglen;

	            if(EMVBase_ParseExtLen(SDOL, &index, &templen))
	            {
	                emvbase_free(pdata);
	                return RLT_ERR_EMV_IccDataFormat;
	            }
	            k = templen;
	            m = sdoldataitem->len;
	            if(sdoldataitem->datafomat & EMVTAGFORMAT_N)	//numeric
	            {
	                if(k >= m)
	                {
	                    if(m)
	                    {
	                        memcpy(&buf[k - m], sdoldataitem->data, m);
	                    }
	                    memcpy(&sdolData[indexOut], buf, k);
	                }
	                else
	                {
	                    if(m)
	                    {
	                        memcpy(buf, sdoldataitem->data, m);
	                    }
	                    memcpy(&sdolData[indexOut], &buf[m - k], k);
	                }
	            }
	            else if(sdoldataitem->datafomat & EMVTAGFORMAT_CN)	//compact numeric
	            {
	                if(m)
	                {
	                    memset(buf, 0xFF, 255);
	                    memcpy(buf, sdoldataitem->data, m);
	                    memcpy(&sdolData[indexOut], buf, k);
	                }
	                else
	                {
	                    memset(buf, 0x00, 255);
	                    memcpy(&sdolData[indexOut], buf, k);
	                }
	            }
	            else	//other formats
	            {
	                if(m)
	                {
	                    memcpy(buf, sdoldataitem->data, m);
	                }
	                memcpy(&sdolData[indexOut], buf, k);
	            }
	            indexOut += k;
	            bInTable = 1;
	        }
		}

        if(!bInTable)
        {
            index += taglen;

            if(EMVBase_ParseExtLen(SDOL, &index, &templen))
            {
                emvbase_free(pdata);
                return RLT_ERR_EMV_IccDataFormat;
            }
            k = templen;
            memcpy(&sdolData[indexOut], buf, k);
            indexOut += k;
        }
    }

	if(SPICondition & 0x01) // ID '0001' is on the Tag9F3E
	{
    	memcpy(&sdolData[indexOut], "\x00\x01\x02", 3);
		indexOut += 3;
		memcpy(&sdolData[indexOut], termCategory, 2);
		indexOut += 2;
	}

    memcpy(apdu_s->Command, "\x80\x1A\x00\x00", 4);
    apdu_s->Lc = indexOut + 2;
    len = 0;
    apdu_s->DataIn[len++] = 0x83;

    if((indexOut & 0x80) > 0)
    {
        (apdu_s->Lc)++;
        apdu_s->DataIn[len++] = 0x81;
        apdu_s->DataIn[len++] = indexOut;
    }
    else
    {
        apdu_s->DataIn[len++] = indexOut;
    }
    memcpy((unsigned char*)&apdu_s->DataIn[len], sdolData, indexOut);
    apdu_s->Le = 256;
    apdu_s->EnableCancel = 1;

    emvbase_free(pdata);
    return RLT_EMV_OK;
}

unsigned char EMVBase_ReadSPIRetData(EMVBASE_SELECT_RET* selectRet, unsigned char* DataOut, unsigned short LenOut, EMVBase_UnionStruct *tempAppUnionStruct)
{
    int matchnum;
    EMVBASE_LISTAPPDATA *tempAppAppData;
    unsigned char *tempappnum;
    unsigned char j, k;
    unsigned short index, indexFCI, indexFCIProp, len, lenFCI, lenFCIProp;
    int indexIssuerDiscret, lenIssuerDiscret;
    int indexAppApp, lenAppApp;
    EMVBASE_TERMAPP tempAppTermApp;
    unsigned short templen;
	unsigned char extendedselect[11] = {0};
	unsigned short extendselectlen = 0;
	unsigned short indexttemp=0,tagLENLen=0,tagLen=0;
	int rfulen=0;

    tempappnum = &(tempAppUnionStruct->EntryPoint->AppListCandidatenum);
    tempAppAppData = tempAppUnionStruct->EntryPoint->AppListCandidate;
    *tempappnum = 0;


    index = 0;

    if(DataOut[index] != 0x6F)
    {
        return RLT_ERR_EMV_IccDataFormat;
    }
    index++;

    if(EMVBase_ParseExtLen(DataOut, &index, &lenFCI))
    {
        return RLT_ERR_EMV_IccDataFormat;
    }
    indexFCI = index;

    while(index < indexFCI + lenFCI)
    {
        if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

        if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
		else if(DataOut[index] == 0x84)
        {
            if(selectRet->DFNameExist == 1) { return RLT_ERR_EMV_IccDataFormat; }
            index++;

			if(EMVBase_ParseExtLen(DataOut, &index, &len))
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
            selectRet->DFNameLen = len;
			if(tempAppUnionStruct->CheckTag84 & 0x02)
			{
				if(selectRet->DFNameLen != 14) { return RLT_ERR_EMV_IccDataFormat; }
			}

            if(selectRet->DFNameLen > 16) { return RLT_ERR_EMV_IccDataFormat; }

            memcpy(selectRet->DFName, DataOut + index ,selectRet->DFNameLen);
			if(tempAppUnionStruct->CheckTag84 & 0x04)
			{
				if(memcmp(selectRet->DFName, "2PAY.SYS.DDF01", 14)) { return RLT_ERR_EMV_IccDataFormat; }
			}
            index += selectRet->DFNameLen;
            selectRet->DFNameExist = 1;
        }
        else if(DataOut[index] == 0xA5)
        {
            selectRet->FCIPropExist = 1;
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &lenFCIProp))
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
            indexFCIProp = index;


            while(index < indexFCIProp + lenFCIProp)
            {
                if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

                if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
                else if(!memcmp(DataOut + index, "\xBF\x0C", 2))
                {
                    if(selectRet->IssuerDiscretExist == 1) { return RLT_ERR_EMV_IccDataFormat; }
                    index += 2;

                    if(EMVBase_ParseExtLen(DataOut, &index, &len))
                    {
                        return RLT_ERR_EMV_IccDataFormat;
                    }

                    if(len > 253) { return RLT_ERR_EMV_IccDataFormat; }
                    selectRet->IssuerDiscretLen = len;
                    selectRet->IssuerDiscretExist = 1;

                    indexIssuerDiscret = index;
                    lenIssuerDiscret = len;

                    while(index < indexIssuerDiscret + lenIssuerDiscret)
                    {
                        if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

                        if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
                        else if(DataOut[index] == 0x61)
                        {
							if((*tempappnum) >= tempAppUnionStruct->EntryPoint->AppListCandidateMaxNum)//���������ø���,���ٸ�ֵ
							{
								k = DataOut[index];
	                            index++;

	                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
	                            {
	                                return RLT_ERR_EMV_IccDataFormat;
	                            }
	                            index += len;
								continue;
							}

                        	selectRet->ApplicationTemplateExist = 1;
                            memset(tempAppAppData + (*tempappnum), 0, sizeof(EMVBASE_LISTAPPDATA));
                            index += 1;

                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
                            {
                                return RLT_ERR_EMV_IccDataFormat;
                            }
                            indexAppApp = index;
                            lenAppApp = len;

                            while(index < indexAppApp + lenAppApp)
                            {
                                if(index >= LenOut) { return RLT_ERR_EMV_IccDataFormat; }

                                if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
                                else if(DataOut[index] == 0x4F)
                                {
                                    index += 1;

                                    if(EMVBase_ParseExtLen(DataOut, &index, &templen))
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }
                                    (tempAppAppData + (*tempappnum))->AIDLen = templen;

                                    if((tempAppAppData + (*tempappnum))->AIDLen > 16) { (tempAppAppData + (*tempappnum))->AIDLen = 0; }

                                    j = (tempAppAppData + (*tempappnum))->AIDLen;
                                    memcpy((tempAppAppData + (*tempappnum))->AID, &DataOut[index], j);
                                    index += templen;
                                }
                                else if(DataOut[index] == 0x50)		//AppLabel
                                {
                                    index += 1;

                                    if(EMVBase_ParseExtLen(DataOut, &index, &templen))
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }
                                    (tempAppAppData + (*tempappnum))->AppLabelLen = templen;

                                    if((tempAppAppData + (*tempappnum))->AppLabelLen > 16) { (tempAppAppData + (*tempappnum))->AppLabelLen = 16; }

                                    j = (tempAppAppData + (*tempappnum))->AppLabelLen;
                                    memcpy((tempAppAppData + (*tempappnum))->AppLabel, DataOut + index, j);
                                    index += templen;
                                }
                                else if(DataOut[index] == 0x87)		//AppPriority
                                {
                                    index += 1;

                                    if(EMVBase_ParseExtLen(DataOut, &index, &templen))
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }

                                    if(templen != 1)
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }
                                    (tempAppAppData + (*tempappnum))->Priority = DataOut[index];
                                    index++;
                                }
								else if(!memcmp(DataOut+index,"\x9F\x2A",2))  //Kernel Identifier
								{
									index+=2;

									if(EMVBase_ParseExtLen(DataOut,&index,&templen))
									{
										return RLT_ERR_EMV_IccDataFormat;
									}

									(tempAppAppData + (*tempappnum))->KernelIdentifierLen  = templen;

									if((tempAppAppData + (*tempappnum))->KernelIdentifierLen > 8)
									{
										(tempAppAppData + (*tempappnum))->KernelIdentifierLen = 8;
									}
									memcpy((tempAppAppData + (*tempappnum))->KernelIdentifier,DataOut+index,(tempAppAppData + (*tempappnum))->KernelIdentifierLen);
									index+=templen;
								}
								else if(!memcmp(DataOut + index, "\x9F\x29", 2))
                                {
                                    index += 2;

                                    if(EMVBase_ParseExtLen(DataOut, &index, &templen))
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }
                                    extendselectlen = templen;

                                    if(extendselectlen <= 11)
                                    {
                                        memcpy(extendedselect, DataOut + index, extendselectlen);
                                    }
                                    else
                                    {
                                        extendselectlen = 0;
                                    }
                                    index += templen;
                                }
								else if(!memcmp(DataOut + index, "\x9F\x0A", 2))
								{
									index+=2;

									if(EMVBase_ParseExtLen(DataOut,&index,&templen))
									{
										return RLT_ERR_EMV_IccDataFormat;
									}

									(tempAppAppData + (*tempappnum))->ASRPDLen  = templen;

									if(templen > 0)
									{
										(tempAppAppData + (*tempappnum))->ASRPD = emvbase_malloc(templen);
										memcpy((tempAppAppData + (*tempappnum))->ASRPD,DataOut+index,templen);

									}
									index+=templen;
								}
                                else if(DataOut[index] == 0x61)
                                {
                                    break;
                                }
                                else
                                {
                                	tagLen=0;
									indexttemp = index;
                                    k = DataOut[index];

                                    if((k & 0x1F) == 0x1F)
                                    {
                                        index++;
										tagLen++;
                                    }
                                    index++;
									tagLen++;

									tagLENLen = index;
                                    if(EMVBase_ParseExtLen(DataOut, &index, &len))
                                    {
                                        return RLT_ERR_EMV_IccDataFormat;
                                    }
									tagLENLen = index-tagLENLen;
									tagLen = (len+tagLENLen+tagLen);
									if(tagLen > 0)
									{
										if (NULL != (tempAppAppData + (*tempappnum))->rfu)
										{
											unsigned char *temp = (tempAppAppData + (*tempappnum))->rfu;

											(tempAppAppData + (*tempappnum))->rfu = emvbase_malloc((tempAppAppData + (*tempappnum))->rfuLen + tagLen);
											if((tempAppAppData + (*tempappnum))->rfu == NULL)
											{
												return RLT_EMV_ERR;
											}
											memcpy((tempAppAppData + (*tempappnum))->rfu,temp,(tempAppAppData + (*tempappnum))->rfuLen);
											emvbase_free(temp);
											temp=NULL;
											memcpy((tempAppAppData + (*tempappnum))->rfu + (tempAppAppData + (*tempappnum))->rfuLen, DataOut+indexttemp, tagLen);
											(tempAppAppData + (*tempappnum))->rfuLen  += tagLen;

										}
										else
										{
											(tempAppAppData + (*tempappnum))->rfu = emvbase_malloc(tagLen);
											if((tempAppAppData + (*tempappnum))->rfu == NULL)
											{
												return RLT_EMV_ERR;
											}
											memcpy((tempAppAppData + (*tempappnum))->rfu,DataOut+indexttemp,tagLen);
											(tempAppAppData + (*tempappnum))->rfuLen  = tagLen;
										}
									}

                                    index += len;
                                }
                            }

                            if((tempAppAppData + (*tempappnum))->AIDLen != 0 && (*tempappnum) < tempAppUnionStruct->EntryPoint->AppListCandidateMaxNum)
                            {
                                matchnum = tempAppUnionStruct->CheckMatchTermAID((tempAppAppData + (*tempappnum))->AID, (tempAppAppData + (*tempappnum))->AIDLen, &tempAppTermApp,(tempAppAppData + (*tempappnum))->KernelIdentifier, (tempAppAppData + (*tempappnum))->KernelIdentifierLen);

                                if(matchnum >= 0)
                                {
                                	if(tempAppUnionStruct->SupportExternSelect && extendselectlen && ((tempAppAppData + (*tempappnum))->AIDLen + extendselectlen) <= 16)
		                            {
		                                memcpy((tempAppAppData + (*tempappnum))->AID + (tempAppAppData + (*tempappnum))->AIDLen, extendedselect, extendselectlen);
		                                (tempAppAppData + (*tempappnum))->AIDLen += extendselectlen;
		                            }

                                    (tempAppAppData + (*tempappnum))->AidInTermLen = tempAppTermApp.AIDLen;
                                    memcpy((tempAppAppData + (*tempappnum))->AidInTerm, tempAppTermApp.AID, (tempAppAppData + (*tempappnum))->AidInTermLen);
                                    (*tempappnum)++;
                                }
                            }

							extendselectlen = 0;
							memset(extendedselect, 0, sizeof(extendedselect));
                        }
                        else if(!memcmp(DataOut + index, "\x9F\x4D", 2))	//Log Entry
                        {
                            if(selectRet->LogEntryExist == 1) { return RLT_ERR_EMV_IccDataFormat; }
                            index += 2;

                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
                            {
                                return RLT_ERR_EMV_IccDataFormat;
                            }
                            selectRet->LogEntryLen = len;
                            index += selectRet->LogEntryLen;
                            selectRet->LogEntryExist = 1;
                        }
                        else
                        {
                            k = DataOut[index];

                            if((k & 0x1F) == 0x1F)
                            {
                                index++;
                            }
                            index++;

                            if(EMVBase_ParseExtLen(DataOut, &index, &len))
                            {
                                return RLT_ERR_EMV_IccDataFormat;
                            }
                            index += len;
                        }
                    }
                }
                else		//other unknown TLV data
                {
                    k = DataOut[index];

                    if((k & 0x1F) == 0x1F)
                    {
                        index++;
                    }
                    index++;

                    if(EMVBase_ParseExtLen(DataOut, &index, &len))
                    {
                        return RLT_ERR_EMV_IccDataFormat;
                    }
                    index += len;
                }
            }

            if(index != indexFCIProp + lenFCIProp)
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
        }
        else
        {
            k = DataOut[index];

            if((k & 0x1F) == 0x1F)
            {
                index++;
            }
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &len))
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
            index += len;
        }
    }

    if(index != indexFCI + lenFCI)
    {
        return RLT_ERR_EMV_IccDataFormat;
    }
    return RLT_EMV_OK;
}

unsigned char EMVBase_SPIRetData(EMVBASE_APDU_RESP *apdu_r, EMVBase_UnionStruct *tempAppUnionStruct)
{
    EMVBASE_SELECT_RET selectRet;

    if(apdu_r->SW1 == 0x90 && apdu_r->SW2 == 0x00)
    {
        memset((unsigned char*)&selectRet, 0, sizeof(EMVBASE_SELECT_RET));

		if(EMVBase_ReadSPIRetData(&selectRet, apdu_r->DataOut, apdu_r->LenOut, tempAppUnionStruct) != RLT_EMV_OK)
		{
			return RLT_ERR_EMV_IccDataFormat;
		}

		if(tempAppUnionStruct->CheckTag84 & 0x01)
		{
			if(selectRet.DFNameExist == 0)
			{
				EMVBase_Trace("EMVBase_SPIRetData-line: %d\r\n", __LINE__);
				return RLT_ERR_EMV_IccDataFormat;
			}
		}

		if(selectRet.ApplicationTemplateExist == 0)
		{
			return RLT_EMV_TERMINATE_TRANSERR;
		}

        if(selectRet.FCIPropExist == 0)
        {
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(selectRet.SFIExist == 1)
        {
            if(selectRet.SFI < 1 || selectRet.SFI > 10 )
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
        }

        return RLT_EMV_OK;
    }
    else if(apdu_r->SW1 == 0x6A && apdu_r->SW2 == 0x81)
    {
        return RLT_ERR_EMV_CardBlock;
    }
	else if(apdu_r->SW1 == 0x6A && apdu_r->SW2 == 0x82)
	{
		return RLT_EMV_PPSE_REV_6A82;
	}
    else
    {
        return RLT_ERR_EMV_IccReturn;
    }
}

unsigned char EMVBase_SelectPPSE(EMVBase_UnionStruct *tempEMVBase_UnionStruct)
{
    EMVBASE_APDU_SEND apdu_s;
    EMVBASE_APDU_RESP apdu_r;
    unsigned char retCode;
	unsigned short index = 0;
	EMVBASETAGCVLITEM *item = NULL;
	unsigned char SPICondition = 0;  // bit1-ID '0001' is on the Tag9F3E,  bit2-SDOL exist
	unsigned char i = 0;
	unsigned char termCategory[2] = {0};
	unsigned char POIInformation[64] = {0}; // var to 64
	unsigned char POIInformationIndex = 0;
	unsigned char POIInformationLen = 0;


	if(tempEMVBase_UnionStruct->rapdu != NULL)
	{
		memcpy(tempEMVBase_UnionStruct->rapdu,"\x00\x00",2);
	}
	memset(termCategory, 0x00, 2);


    EMVBase_COMMAND_SELECT("2PAY.SYS.DDF01", 14, 0, &apdu_s);

    tempEMVBase_UnionStruct->IsoCommand(&apdu_s, &apdu_r);

    if(apdu_r.ReadCardDataOk != RLT_EMV_OK)
    {
        return RLT_ERR_EMV_APDUTIMEOUT;
    }

	if(tempEMVBase_UnionStruct->rapdu != NULL)
	{
		tempEMVBase_UnionStruct->rapdu[0]  = (apdu_r.LenOut + 2) / 256;
		tempEMVBase_UnionStruct->rapdu[1]  = (apdu_r.LenOut + 2) % 256;
		memcpy(tempEMVBase_UnionStruct->rapdu + 2,apdu_r.DataOut,apdu_r.LenOut);
		tempEMVBase_UnionStruct->rapdu[apdu_r.LenOut + 2]  = apdu_r.SW1;
		tempEMVBase_UnionStruct->rapdu[apdu_r.LenOut + 3]  = apdu_r.SW2;
	}

    retCode = EMVBase_SelectDFRetData(&apdu_r, tempEMVBase_UnionStruct);
	EMVBase_Trace("EMVBase_SelectDFRetData-retCode: %d\r\n", retCode);


	if(1 == tempEMVBase_UnionStruct->SupportSPICommand)
	{
		item = emvbase_avl_gettagitempointer(EMVTAG_POIInformation);
		if((item != NULL) && item->len)
		{
			memset(POIInformation, 0x00, sizeof(POIInformation));
			POIInformationLen = item->len;
			memcpy(POIInformation, item->data, item->len);

			item = emvbase_avl_gettagitempointer(EMVTAG_TermCateSupportedList);
			if(item != NULL)
		    {
		        if(item->len % 2)
		        {
		            return RLT_ERR_EMV_IccDataFormat;
		        }

				POIInformationIndex = 0;
				while(POIInformationIndex < POIInformationLen)
				{
					if((0 == memcmp(POIInformation+POIInformationIndex, "\x00\x01", 2)) && (2 == POIInformation[POIInformationIndex+2])) // ID==0001 AND L==2
					{
						POIInformationIndex += 3; // ID+L
						memcpy(termCategory, POIInformation+POIInformationIndex, 2);
						POIInformationIndex += 2; // V
					}
					else
					{
						POIInformationIndex += 3;
						POIInformationIndex += POIInformation[POIInformationIndex-1];
						continue;
					}

					if(0 != memcmp(termCategory, "\x00\x00", 2))
					{
						index = 0;
						while(index < item->len)
						{
							if(0 == memcmp(item->data+index, termCategory, 2))
							{
								EMVBase_Trace("EP-info: the Terminal Category (POI Information ID '0001') of the terminal is on the TerminalCategories Supported List\r\n");
								SPICondition |= 0x01;
								break;
							}

							index += 2;
						}
					}

					if(SPICondition)
					{
						break;
					}
				}
		    }
		}
		else
		{
			//return retCode;
		}


		item = emvbase_avl_gettagitempointer(EMVTAG_SDOL);
		if(item != NULL)
	    {
	        if(item->len)
	        {
	        	EMVBase_Trace("EP-info: the SDOL(Tag9F3F) is returned in the FCI\r\n");
	            SPICondition |= 0x02;
	        }
	    }

		if(SPICondition)
		{
			for(i=0; i<tempEMVBase_UnionStruct->EntryPoint->AppListCandidateMaxNum; i++)
			{
				if(tempEMVBase_UnionStruct->EntryPoint->AppListCandidate[i].ASRPDLen && tempEMVBase_UnionStruct->EntryPoint->AppListCandidate[i].ASRPD)
				{
					emvbase_free(tempEMVBase_UnionStruct->EntryPoint->AppListCandidate[i].ASRPD);
					tempEMVBase_UnionStruct->EntryPoint->AppListCandidate[i].ASRPDLen = 0;
					tempEMVBase_UnionStruct->EntryPoint->AppListCandidate[i].ASRPD = NULL;
				}
			}
			memset(tempEMVBase_UnionStruct->EntryPoint->AppListCandidate, 0, sizeof(EMVBASE_LISTAPPDATA) * tempEMVBase_UnionStruct->EntryPoint->AppListCandidateMaxNum);
			tempEMVBase_UnionStruct->EntryPoint->AppListCandidatenum = 0;


			retCode = EMVBase_SDOLProcess(tempEMVBase_UnionStruct, &apdu_s, SPICondition, termCategory);
		    if(retCode != RLT_EMV_OK)
		    {
		        return retCode;
		    }

			tempEMVBase_UnionStruct->IsoCommand(&apdu_s, &apdu_r);

		    if(apdu_r.ReadCardDataOk != RLT_EMV_OK)
		    {
		        return RLT_ERR_EMV_APDUTIMEOUT;
		    }

			retCode = EMVBase_SPIRetData(&apdu_r, tempEMVBase_UnionStruct);
		}
	}

    return retCode;
}

unsigned char EMVBase_ChooseApp(EMVBase_UnionStruct *tempEMVBase_UnionStruct)
{
    unsigned char i, j, temp;
    unsigned char *tempselectappno;
    unsigned char *tempappnum;
    unsigned char noprioritynum = 0;
    unsigned char *tmpseq = NULL;
    unsigned char *tmpapprtiority = NULL;
    unsigned char *AppPriority = NULL;
    unsigned char *AppSeq = NULL;
    EMVBASE_LISTAPPDATA *tempAppAppData;


    tempselectappno = &(tempEMVBase_UnionStruct->SelectedAppNo);
    tempappnum = &(tempEMVBase_UnionStruct->EntryPoint->AppListCandidatenum);
    tempAppAppData = tempEMVBase_UnionStruct->EntryPoint->AppListCandidate;


    if(*tempappnum < 1)
    {
        return RLT_ERR_EMV_NoAppSel;
    }
    *tempselectappno = 0xff;

	tmpseq = emvbase_malloc(tempEMVBase_UnionStruct->EntryPoint->AppListCandidateMaxNum);
	tmpapprtiority = emvbase_malloc(tempEMVBase_UnionStruct->EntryPoint->AppListCandidateMaxNum);
	AppPriority = emvbase_malloc(tempEMVBase_UnionStruct->EntryPoint->AppListCandidateMaxNum);
	AppSeq = emvbase_malloc(tempEMVBase_UnionStruct->EntryPoint->AppListCandidateMaxNum);

	if(tmpseq==NULL || tmpapprtiority==NULL || AppPriority==NULL || AppSeq==NULL)
	{
		if(tmpseq) emvbase_free(tmpseq);
		if(tmpapprtiority) emvbase_free(tmpapprtiority);
		if(AppPriority) emvbase_free(AppPriority);
		if(AppSeq) emvbase_free(AppSeq);
		return RLT_EMV_ERR;
	}


    if(*tempselectappno > *tempappnum)
    {
        for(i = 0; i < *tempappnum; i++)
        {
            AppPriority[i] = (tempAppAppData + i)->Priority & 0x0F;
            AppSeq[i] = i;
        }

        if(*tempappnum == 1)
        {
            *tempselectappno = 0;
        }
        else
        {
            for(j = 0; j < *tempappnum - 1; j++)
            {
                for(i = 0; i < *tempappnum - 1 - j; i++)
                {
                    if(AppPriority[i] > AppPriority[i + 1])
                    {
                        temp = AppPriority[i];
                        AppPriority[i] = AppPriority[i + 1];
                        AppPriority[i + 1] = temp;
                        temp = AppSeq[i];
                        AppSeq[i] = AppSeq[i + 1];
                        AppSeq[i + 1] = temp;
                    }
                }
            }

            noprioritynum = 0;

            for(i = 0; i < *tempappnum; i++)
            {
                if(AppPriority[i] == 0) { noprioritynum++; }
            }

            if(noprioritynum)              //have no priority app
            {
                memcpy(tmpapprtiority, &AppPriority[noprioritynum], *tempappnum - noprioritynum);
                memcpy(&tmpapprtiority[*tempappnum - noprioritynum], AppPriority, noprioritynum);
                memcpy(AppPriority, tmpapprtiority, *tempappnum);
                memcpy(tmpseq, &AppSeq[noprioritynum], *tempappnum - noprioritynum);
                memcpy(&tmpseq[*tempappnum - noprioritynum], AppSeq, noprioritynum);
                memcpy(AppSeq, tmpseq, *tempappnum);
            }
            *tempselectappno = AppSeq[0];
        }
    }

	if(tmpseq) emvbase_free(tmpseq);
	if(tmpapprtiority) emvbase_free(tmpapprtiority);
	if(AppPriority) emvbase_free(AppPriority);
	if(AppSeq) emvbase_free(AppSeq);

	return RLT_EMV_OK;
}

void EMVBase_AppCopy(unsigned char i, unsigned char j, EMVBASE_LISTAPPDATA *ListCandidate)
{
    memcpy((unsigned char*)&ListCandidate[i], (unsigned char*)&ListCandidate[j], sizeof(EMVBASE_LISTAPPDATA));
}

unsigned char EMVBase_RebuildApplist(EMVBase_UnionStruct *tempEMVBase_UnionStruct)
{
    unsigned char i;
    EMVBASE_LISTAPPDATA *tempapplist;

    tempapplist = tempEMVBase_UnionStruct->EntryPoint->AppListCandidate;

    if((tempEMVBase_UnionStruct->EntryPoint->AppListCandidatenum) > 1)
    {
        for(i = (tempEMVBase_UnionStruct->SelectedAppNo); i < (tempEMVBase_UnionStruct->EntryPoint->AppListCandidatenum) - 1; i++)
        {
            EMVBase_AppCopy(i, i + 1, tempapplist);
        }

        (tempEMVBase_UnionStruct->EntryPoint->AppListCandidatenum) --;
        return RLT_EMV_OK;
    }
    else
    {
        return RLT_ERR_EMV_NoAppSel;
    }
}

unsigned char EMVBase_CheckifRedundantData(unsigned char *tag, unsigned char *rundatabuf, unsigned int rundatalen)
{
    unsigned int index = 0;
    unsigned char temptag[4], temptaglen;
    EMVBASETAGCVLITEM tempemvtagitem;

    while(index < rundatalen)
    {
        temptaglen = 0;

        temptag[temptaglen++] = rundatabuf[index];

        if((temptag[0] & 0x1f) == 0x1f)
        {
            temptag[temptaglen++] = rundatabuf[index + 1];

            if(tag[1] & 0x80)
            {
                temptag[temptaglen++] = rundatabuf[index + 2];
            }
        }

        if(memcmp(tag, temptag, temptaglen) == 0)
        {
            return 1;
        }
        index += temptaglen;
    }

    return 0;
}


unsigned char EMVBase_CheckDateFormat(unsigned char * date)
{
	unsigned char i,k,n[3];
	for(i=0;i<3;i++){
		k=date[i];
		if((k&0x0F) > 9) return RLT_EMV_ERR;
		if(((k&0xF0)>>4) > 9) return RLT_EMV_ERR;
		n[i]=((k&0xF0)>>4)*10 + (k&0x0F);
	}
	if(n[1]<1 || n[1]>12) return RLT_EMV_ERR;
	if(n[2]<1 || n[2]>31) return RLT_EMV_ERR;
	if(n[1]==0x02)
	{
		if(n[2]>29) return RLT_EMV_ERR;
	}
	return RLT_EMV_OK;
}


void EMVBase_FillIPK(unsigned char* ipkData, EMVBASE_IPK_RECOVER* recovIPK, EMVBASE_CAPK_STRUCT *tempcapk)
{
    unsigned char i;

    recovIPK->DataHead = ipkData[0];
    recovIPK->CertFormat = ipkData[1];
    memcpy(recovIPK->IssuID, (unsigned char*)&ipkData[2], 4);
    memcpy(recovIPK->ExpireDate, (unsigned char*)&ipkData[6], 2);
    memcpy(recovIPK->CertSerial, (unsigned char*)&ipkData[8], 3);
    recovIPK->HashInd = ipkData[11];
    recovIPK->IPKAlgoInd = ipkData[12];
    recovIPK->IPKLen = ipkData[13];
    recovIPK->IPKExpLen = ipkData[14];

    memcpy((unsigned char*)&recovIPK->IPKLeft, (unsigned char*)&ipkData[15], tempcapk->ModulLen - 36);

    for(i = 0; i < 20; i++)
    {
        recovIPK->HashResult[i] = ipkData[tempcapk->ModulLen - 21 + i];
    }

    recovIPK->DataTrail = ipkData[tempcapk->ModulLen - 1];
}


void EMVBase_FillICCPK(unsigned char* ICCPKData, unsigned char tempIPKModulLen, EMVBASE_ICCPK_RECOVER* recovICCPK)
{
    unsigned char i;

    recovICCPK->DataHead = ICCPKData[0];
    recovICCPK->CertFormat = ICCPKData[1];
    memcpy(recovICCPK->AppPAN, (unsigned char*)&ICCPKData[2], 10);
    memcpy(recovICCPK->ExpireDate, (unsigned char*)&ICCPKData[12], 2);
    memcpy(recovICCPK->CertSerial, (unsigned char*)&ICCPKData[14], 3);
    recovICCPK->HashInd = ICCPKData[17];
    recovICCPK->ICCPKAlgoInd = ICCPKData[18];
    recovICCPK->ICCPKLen = ICCPKData[19];
    recovICCPK->ICCPKExpLen = ICCPKData[20];
    memcpy(recovICCPK->ICCPKLeft, (unsigned char*)&ICCPKData[21], tempIPKModulLen - 42);

    for(i = 0; i < 20; i++)
    {
        recovICCPK->HashResult[i] = ICCPKData[tempIPKModulLen - 21 + i];
    }

    recovICCPK->DataTrail = ICCPKData[tempIPKModulLen - 1];
}


void EMVBase_FillSSA(unsigned char* SSAData, EMVBASE_SIGN_STATDATA_RECOVER* recovSSA, unsigned char tempIPKModulLen)
{
    unsigned char i;

    recovSSA->DataHead = SSAData[0];
    recovSSA->DataFormat = SSAData[1];
    recovSSA->HashInd = SSAData[2];
    memcpy(recovSSA->DataAuthCode, (unsigned char*)&SSAData[3], 2);
    //memcpy(recovSSA->PadPattern, (unsigned char*)&SSAData[5], tempIPKModulLen - 26);

    for(i = 0; i < 20; i++)
    {
        recovSSA->HashResult[i] = SSAData[tempIPKModulLen - 21 + i];
    }

    recovSSA->DataTrail = SSAData[tempIPKModulLen - 1];
}

unsigned char EMVBase_FillSDA(unsigned char* SDAData, EMVBASE_SIGN_DYNDATA_RECOVER* recovSDA, unsigned char tempICCPKModulLen)
{
    unsigned char LDD, i;

    recovSDA->DataHead = SDAData[0];
    recovSDA->DataFormat = SDAData[1];
    recovSDA->HashInd = SDAData[2];
    recovSDA->ICCDynDataLen = SDAData[3];
    LDD = SDAData[3];

    if(LDD > tempICCPKModulLen - 25)
    {
        return RLT_EMV_ERR;
    }
    memcpy(recovSDA->ICCDynData, (unsigned char*)&SDAData[4], LDD);
    //memcpy(recovSDA->PadPattern, (unsigned char*)&SDAData[LDD + 4], tempICCPKModulLen - LDD - 25);

    for(i = 0; i < 20; i++)
    {
        recovSDA->HashResult[i] = SDAData[tempICCPKModulLen - 21 + i];
    }

    recovSDA->DataTrail = SDAData[tempICCPKModulLen - 1];
    return RLT_EMV_OK;
}


unsigned char EMVBase_SM_FillIPK(EMVBASE_SM_IPK_CRTFCT *IPKCertfct, unsigned char *IPKCert, unsigned char IPKCertLen, EMVBASE_CAPK_STRUCT *tempcapk)
{
    unsigned char i = 0;

    IPKCertfct->CertFormat = IPKCert[i++];
    memcpy(IPKCertfct->IssuID, (unsigned char*)&IPKCert[i], 4);
    i += 4;
    memcpy(IPKCertfct->ExpireDate, (unsigned char*)&IPKCert[i], 2);
    i += 2;
    memcpy(IPKCertfct->CertSerial, (unsigned char*)&IPKCert[i], 3);
    i += 3;
    IPKCertfct->IPKSIGNAlgoInd = IPKCert[i++];
    IPKCertfct->IPKEncrptAlgoInd = IPKCert[i++];
    IPKCertfct->IPKParamInd = IPKCert[i++];

    IPKCertfct->IPKLen = IPKCert[i++];
	if(IPKCertfct->IPKLen != 64)
	{
        return RLT_EMV_ERR;
    }
    memcpy((unsigned char*)&IPKCertfct->IPK, (unsigned char*)&IPKCert[i], IPKCertfct->IPKLen);
    i += IPKCertfct->IPKLen;
    memcpy((unsigned char*)&IPKCertfct->DGTLSGNTR, (unsigned char*)&IPKCert[i], 64);
    i += tempcapk->ModulLen;

    if(i != IPKCertLen)
    {
        return RLT_EMV_ERR;
    }
    return RLT_EMV_OK;
}


unsigned char EMVBase_SM_FillICCPK(EMVBASE_SM_ICCPK_CRTFCT *ICCPKCertfct, unsigned char *ICCPKCert, unsigned char ICCPKCertLen)
{
    unsigned char i = 0;

    ICCPKCertfct->CertFormat = ICCPKCert[i++];
    memcpy(ICCPKCertfct->AppPAN, (unsigned char*)&ICCPKCert[i], 10);
    i += 10;
    memcpy(ICCPKCertfct->ExpireDate, (unsigned char*)&ICCPKCert[i], 2);
    i += 2;
    memcpy(ICCPKCertfct->CertSerial, (unsigned char*)&ICCPKCert[i], 3);
    i += 3;
    ICCPKCertfct->ICCPKSIGNAlgoInd = ICCPKCert[i++];
    ICCPKCertfct->ICCPKEncrptAlgoInd = ICCPKCert[i++];
    ICCPKCertfct->ICCPKParamInd = ICCPKCert[i++];
    ICCPKCertfct->ICCPKLen = ICCPKCert[i++];

	if(ICCPKCertfct->ICCPKLen != 64)
	{
        return RLT_EMV_ERR;
    }

    memcpy((unsigned char*)&ICCPKCertfct->ICCPK, (unsigned char*)&ICCPKCert[i], ICCPKCertfct->ICCPKLen);
    i += ICCPKCertfct->ICCPKLen;
    memcpy((unsigned char*)&ICCPKCertfct->DGTLSGNTR, (unsigned char*)&ICCPKCert[i], 64);
    i += 64;

    if(i != ICCPKCertLen)
    {
        return RLT_EMV_ERR;
    }
    return RLT_EMV_OK;
}



unsigned char EMVBase_ExpireDateVerify(unsigned char* ExpireDate)
{
    unsigned char CurDate[3], ExpDate[3];

    emvbase_avl_gettagvalue_spec(EMVTAG_TransDate, &CurDate[1], 0, 2);

    if(CurDate[1] > 0x49) { CurDate[0] = 0x19; }
    else{ CurDate[0] = 0x20; }

    if(ExpireDate[1] > 0x49) { ExpDate[0] = 0x19; }
    else{ ExpDate[0] = 0x20; }
    ExpDate[1] = ExpireDate[1];
    ExpDate[2] = ExpireDate[0];

    if(memcmp(CurDate, ExpDate, 3) > 0) //expired
    {
    	EMVBase_Trace(" Date Expire err !!! line=%d\r\n",__LINE__);
		EMVBase_TraceHex("CurDate",CurDate,3);
		EMVBase_TraceHex("ExpDate",ExpDate,3);
        return RLT_EMV_ERR;
    }
    else
    {
        return RLT_EMV_OK;
    }
}

void EMVBase_COMMAND_SELECT(unsigned char *DFFileName, unsigned char DFFileNameLen, unsigned char nextflag, EMVBASE_APDU_SEND *apdu_s)
{
    if(nextflag)
    {
        memcpy(apdu_s->Command, "\x00\xA4\x04\x02", 4);
    }
    else
    {
        memcpy(apdu_s->Command, "\x00\xA4\x04\x00", 4);
    }
    apdu_s->Lc = DFFileNameLen;
    memcpy(apdu_s->DataIn, DFFileName, DFFileNameLen);
    apdu_s->Le = 256;
    apdu_s->EnableCancel = 1;
}

unsigned char EMVBase_CheckReadAFL(EMVBASETAGCVLITEM *item)
{
    unsigned short t, i, j;
    unsigned char AFL_Num;
    unsigned char *AFL;


    AFL_Num = item->len >> 2;

    if(AFL_Num == 0)
    {
        return RLT_ERR_EMV_IccDataFormat;
    }
    AFL = item->data;



    for(i = 0; i < AFL_Num; i++)
    {
        j = i << 2;

        t = AFL[j];
        t >>= 3;

        if(t == 0 || t >= 31)
        {
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(AFL[j + 1] < 1)
        {
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(AFL[j + 2] < AFL[j + 1])
        {
            return RLT_ERR_EMV_IccDataFormat;
        }

        if(AFL[j + 3] > (AFL[j + 2] - AFL[j + 1] + 1))
        {
            return RLT_ERR_EMV_IccDataFormat;
        }
    }

    return RLT_EMV_OK;
}

void EMVBase_FormReadAflData(EMVBASE_APDU_SEND *apdu_s, unsigned char tempAFL_RecordNum, unsigned char SFI)
{
    memcpy(apdu_s->Command, "\x00\xB2", 2);
    apdu_s->Command[2] = tempAFL_RecordNum;     //record number
    apdu_s->Command[3] = (SFI & 0xF8) | 0x04; 	//SFI
    apdu_s->Lc = 0;
    apdu_s->Le = 256;

    apdu_s->EnableCancel = 1;
}

void EMVBase_PostReadApp(unsigned short i, unsigned char nextflag, EMVBASE_TERMAPP *temptermapp, EMVBASE_APDU_SEND *apdu_s)
{
    if(nextflag)
    {
        memcpy(apdu_s->Command, "\x00\xA4\x04\x02", 4);       //ADF,select next app with same AID
    }
    else
    {
        memcpy(apdu_s->Command, "\x00\xA4\x04\x00", 4);       //ADF
    }
    apdu_s->Lc = (temptermapp + i)->AIDLen;

    memcpy(apdu_s->DataIn, (temptermapp + i)->AID, (temptermapp + i)->AIDLen);
    apdu_s->Le = 256;
    apdu_s->EnableCancel = 1;
}

unsigned char EMVBase_ReadSelectTermRetData(EMVBASE_SELECT_RET* selectRet,unsigned char* DataOut,unsigned short LenOut)
{
	unsigned char j,k;
	unsigned short index,indexFCI,indexFCIProp,len,lenFCI,lenFCIProp;
	int indexIssuerDiscret,lenIssuerDiscret;


	index=0;
	if(DataOut[index]!=0x6F)	//FCIģ���ʶ
	{
		return RLT_ERR_EMV_IccDataFormat; //FCI template
	}
	index++;

    if(EMVBase_ParseExtLen(DataOut,&index,&lenFCI))
    {
    	return RLT_ERR_EMV_IccDataFormat;
    }
	indexFCI=index;

	while(index<indexFCI+lenFCI)
	{
		if(index>=LenOut) return RLT_ERR_EMV_IccDataFormat;
		if(DataOut[index]==0xFF || DataOut[index]==0x00) { index++; continue;}
		else if(DataOut[index]==0x84)	//DF��
		{
			if(selectRet->DFNameExist==1) return RLT_ERR_EMV_IccDataFormat;
			index++;
			selectRet->DFNameLen=DataOut[index];
			if(selectRet->DFNameLen>16) return RLT_ERR_EMV_IccDataFormat;
			memcpy(selectRet->DFName,DataOut+index+1,DataOut[index]);
			index+=selectRet->DFNameLen+1;
			selectRet->DFNameExist=1;
		}
		else if(!memcmp(DataOut+index,"\x9F\x11",2)) //ICTI
		{
		    if(selectRet->ICTIExist)
		    {
		    	return RLT_ERR_EMV_IccDataFormat;
		    }
            j = DataOut[index+2];
			index+=3;
			selectRet->ICTI=DataOut[index];
			index+=j;
			selectRet->ICTIExist=1;
		}
		else if(!memcmp(DataOut+index,"\x9F\x12",2)) //App Prefer Name
		{
		    if(selectRet->PreferNameExist)
		    {
		    	return RLT_ERR_EMV_IccDataFormat;
		    }
			index+=2;
			selectRet->PreferNameLen=DataOut[index];
			j = DataOut[index];
			if(selectRet->PreferNameLen > 16) selectRet->PreferNameLen = 16;
			index++;
			memcpy(selectRet->PreferName,DataOut+index,selectRet->PreferNameLen);
			index+=j;
			selectRet->PreferNameExist=1;
		}
		else if(DataOut[index]==0xA5)	//FCI����ר��ģ��
		{
			selectRet->FCIPropExist=1;
			index++;

			if(EMVBase_ParseExtLen(DataOut,&index,&lenFCIProp))
			{
				return RLT_ERR_EMV_IccDataFormat;
			}
			indexFCIProp=index;


			while(index<indexFCIProp+lenFCIProp)
			{
				if(index>=LenOut) return RLT_ERR_EMV_IccDataFormat;
				if(DataOut[index]==0xFF || DataOut[index]==0x00) { index++; continue;}
				else if(DataOut[index]==0x88)//SFI�ļ�
				{
					if(selectRet->SFIExist==1) return RLT_ERR_EMV_IccDataFormat;
					if(DataOut[index+1]!=1) return RLT_ERR_EMV_IccDataFormat;
					index+=2;
					selectRet->SFI=DataOut[index];
					index++;
					selectRet->SFIExist=1;
				}
				else if(!memcmp(DataOut+index,"\x5F\x2D",2)) //Language preference
				{
					index+=2;
					selectRet->LangPreferLen=DataOut[index];
					j = DataOut[index];
					if(selectRet->LangPreferLen > 8) selectRet->LangPreferLen = 8;
					index++;
					memcpy(selectRet->LangPrefer,DataOut+index,selectRet->LangPreferLen);
					index+=j;
					selectRet->LangPreferExist=1;
				}
				else if(!memcmp(DataOut+index,"\x9F\x11",2)) //ICTI
				{
				    if(selectRet->ICTIExist)
				    {
				    	return RLT_ERR_EMV_IccDataFormat;
				    }
                    j = DataOut[index+2];
					index+=3;
					selectRet->ICTI=DataOut[index];
					index+=j;
					selectRet->ICTIExist=1;
				}
				else if(DataOut[index]==0x50) //App Label
				{
				    index++;
					selectRet->AppLabelLen=DataOut[index];
					j= DataOut[index];
					if(selectRet->AppLabelLen > 16) selectRet->AppLabelLen = 16;
					index++;
					memcpy(selectRet->AppLabel,DataOut+index,selectRet->AppLabelLen);
					index+=j;
					selectRet->AppLabelExist=1;
				}
				else if(DataOut[index]==0x87)//App Priority Indicator
				{
					if(selectRet->PriorityExist==1) return RLT_ERR_EMV_IccDataFormat;
					if(DataOut[index+1]!=1) return RLT_ERR_EMV_IccDataFormat;
					index+=2;
					selectRet->Priority=DataOut[index];
					index++;
					selectRet->PriorityExist=1;
				}
				else if(!memcmp(DataOut+index,"\x9F\x38",2)) //PDOL
				{
					if(selectRet->PDOLExist==1) return RLT_ERR_EMV_IccDataFormat;
					index+=2;
					if(EMVBase_ParseExtLen(DataOut,&index,&len))
					{
						return RLT_ERR_EMV_IccDataFormat;
					}

					selectRet->PDOLLen=len;
					index+=selectRet->PDOLLen;
					selectRet->PDOLExist=1;
				}
				else if(!memcmp(DataOut+index,"\x9F\x12",2)) //App Prefer Name
				{
				    if(selectRet->PreferNameExist)
				    {
				    	return RLT_ERR_EMV_IccDataFormat;
				    }
					index+=2;
					selectRet->PreferNameLen=DataOut[index];
					j = DataOut[index];
					if(selectRet->PreferNameLen > 16) selectRet->PreferNameLen = 16;
					index++;
					memcpy(selectRet->PreferName,DataOut+index,selectRet->PreferNameLen);
					index+=j;
					selectRet->PreferNameExist=1;
				}
				else if(!memcmp(DataOut+index,"\xBF\x0C",2)) //FCI Issuer Discretionary Data
				{
					if(selectRet->IssuerDiscretExist==1) return RLT_ERR_EMV_IccDataFormat;
					index+=2;
					if(EMVBase_ParseExtLen(DataOut,&index,&len))
					{
						return RLT_ERR_EMV_IccDataFormat;
					}
					selectRet->IssuerDiscretLen=len;
					selectRet->IssuerDiscretExist=1;

					indexIssuerDiscret=index;
					lenIssuerDiscret=len;
					while(index<indexIssuerDiscret+lenIssuerDiscret)
					{
						if(index>=LenOut) return RLT_ERR_EMV_IccDataFormat;
						if(DataOut[index]==0xFF || DataOut[index]==0x00) { index++; continue;}
						else if(!memcmp(DataOut+index,"\x9F\x4D",2))//Log Entry
						{
							if(selectRet->LogEntryExist==1) return RLT_ERR_EMV_IccDataFormat;
							index+=2;
							if(EMVBase_ParseExtLen(DataOut,&index,&len))
							{
								return RLT_ERR_EMV_IccDataFormat;
							}
							selectRet->LogEntryLen=len;
							index+=selectRet->LogEntryLen;
							selectRet->LogEntryExist=1;
						}
						else
						{
							k=DataOut[index];
							if((k&0x1F)==0x1F)
								index++;
							index++;
							if(EMVBase_ParseExtLen(DataOut,&index,&len))
							{
								return RLT_ERR_EMV_IccDataFormat;
							}
							index+=len;
						}

					}

				}
				else//other unknown TLV data
				{
					k=DataOut[index];
					if((k&0x1F)==0x1F)
						index++;
					index++;
					if(EMVBase_ParseExtLen(DataOut,&index,&len))
					{
						return RLT_ERR_EMV_IccDataFormat;
					}
					index+=len;
				}
			}
			if(index!=indexFCIProp+lenFCIProp)
				return RLT_ERR_EMV_IccDataFormat;
		}
		else if(!memcmp(DataOut+index,"\x9F\x38",2))
		{
			return RLT_ERR_EMV_IccDataFormat;
		}
		else
		{
			k=DataOut[index];
			if((k&0x1F)==0x1F)
				index++;
			index++;
			if(EMVBase_ParseExtLen(DataOut,&index,&len))
			{
				return RLT_ERR_EMV_IccDataFormat;
			}
			index+=len;
		}

	}
	if(index!=indexFCI+lenFCI)
		return RLT_ERR_EMV_IccDataFormat;

	return RLT_EMV_OK;

}

unsigned char EMVBase_SelectTermRetData(EMVBASE_APDU_RESP *apdu_r,EMVBase_TermAidlistUnionStruct *temp_UnionStruct,unsigned char appnumindex,EMVBASE_TERMAPP *temptermapp)
{
    EMVBASE_SELECT_RET selectRet;
    unsigned char retCode;

    EMVBASE_LISTAPPDATA *tempAppData;
	unsigned char *tempapplistnum;


	tempAppData = temp_UnionStruct->AppListCandidate;
	tempapplistnum = &(temp_UnionStruct->AppListCandidatenum);


	if(apdu_r->SW1==0x90 && apdu_r->SW2==0x00) //Select successful
	{
		memset((unsigned char*)&selectRet,0,sizeof(EMVBASE_SELECT_RET));
		retCode=EMVBase_ReadSelectTermRetData(&selectRet,apdu_r->DataOut,apdu_r->LenOut);
		if(retCode!=RLT_EMV_OK)
		{
			return RLT_ERR_EMV_IccDataFormat;
		}

		if(selectRet.DFNameExist==0 || selectRet.FCIPropExist==0)
		{
			return RLT_ERR_EMV_IccDataFormat;
		}
		if(!memcmp((temptermapp+appnumindex)->AID,selectRet.DFName,(temptermapp+appnumindex)->AIDLen))
		{
			if((temptermapp+appnumindex)->AIDLen==selectRet.DFNameLen)
			{
				retCode = RLT_EMV_EXTRACTMATCH;
				(tempAppData+*tempapplistnum)->AidInTermLen = (temptermapp+appnumindex)->AIDLen;
				memcpy((tempAppData+*tempapplistnum)->AidInTerm,(temptermapp+appnumindex)->AID,(tempAppData+*tempapplistnum)->AidInTermLen);
			}
			else
			{
			    retCode = RLT_EMV_PARTICALMATCH;
				if((temptermapp+appnumindex)->ASI!=AID_PARTIAL_MATCH)
				{
					retCode = RLT_EMV_NOTMATCH;
					return retCode;
				}

				(tempAppData+*tempapplistnum)->AidInTermLen = (temptermapp+appnumindex)->AIDLen;
				memcpy((tempAppData+*tempapplistnum)->AidInTerm,(temptermapp+appnumindex)->AID,(tempAppData+*tempapplistnum)->AidInTermLen);
			}
		}
		else
		{
			return RLT_ERR_EMV_IccDataFormat;
		}

		memcpy((tempAppData+*tempapplistnum)->AID,selectRet.DFName,selectRet.DFNameLen);
		(tempAppData+*tempapplistnum)->AIDLen=selectRet.DFNameLen;


		memcpy((tempAppData+*tempapplistnum)->AppLabel,selectRet.AppLabel,selectRet.AppLabelLen);
		(tempAppData+*tempapplistnum)->AppLabelLen=selectRet.AppLabelLen;

		if(selectRet.PriorityExist==1)
		(tempAppData+*tempapplistnum)->Priority=selectRet.Priority;

		if(selectRet.LangPreferExist==1)
		{
			memcpy((tempAppData+*tempapplistnum)->LangPrefer,selectRet.LangPrefer,selectRet.LangPreferLen);
			(tempAppData+*tempapplistnum)->LangPreferLen=selectRet.LangPreferLen;
		}

		if(selectRet.ICTIExist==1)
		{
			(tempAppData+*tempapplistnum)->ICTI=selectRet.ICTI;
			(tempAppData+*tempapplistnum)->ICTILen = 1;
		}
		if(selectRet.PreferNameExist==1)
		{
			memcpy((tempAppData+*tempapplistnum)->PreferName,selectRet.PreferName,selectRet.PreferNameLen);
			(tempAppData+*tempapplistnum)->PreferNameLen=selectRet.PreferNameLen;
		}

		return retCode;
	}
	else
	{
		return RLT_EMV_OK;
	}
}

unsigned char EMVBase_SelectFromTerm(EMVBase_TermAidlistUnionStruct *temp_UnionStruct)
{
    unsigned char retCode;
    unsigned char AppNumIndex = 0;
    EMVBASE_TERMAPP *temptermapp;
	unsigned char temptermlistnum;
	EMVBASE_APDU_SEND apdu_s;
	EMVBASE_APDU_RESP apdu_r;
	EMVBASE_LISTAPPDATA *tempAppData;
	unsigned char bSecond = 0;
    unsigned char *tempapplistnum;


    tempAppData = temp_UnionStruct->AppListCandidate;
	tempapplistnum = &(temp_UnionStruct->AppListCandidatenum);
    temptermapp = (EMVBASE_TERMAPP *)emvbase_malloc(sizeof(EMVBASE_TERMAPP)*100);


	temp_UnionStruct->ReadTermAID(&temptermapp[0],&temptermlistnum);

	EMVBase_Trace("aid num in terminal is %d\r\n",temptermlistnum);

	if(temptermlistnum < 1)
	{
		emvbase_free(temptermapp);
		return RLT_ERR_EMV_NoAppSel;
	}

	while(AppNumIndex < temptermlistnum)
	{
		EMVBase_Trace("aid num in card is %d\r\n",(*tempapplistnum));

	    if((*tempapplistnum) >= temp_UnionStruct->AppListCandidateMaxNum)
    	{
			break;
    	}

		EMVBase_PostReadApp(AppNumIndex,bSecond,temptermapp,&apdu_s);

		temp_UnionStruct->IsoCommand(&apdu_s,&apdu_r);
		if(apdu_r.ReadCardDataOk != RLT_EMV_OK)
		{
		    emvbase_free(temptermapp);
			return RLT_ERR_EMV_APDUTIMEOUT;
		}

		retCode = EMVBase_SelectTermRetData(&apdu_r,temp_UnionStruct,AppNumIndex,&temptermapp[0]);

		if(retCode == RLT_EMV_EXTRACTMATCH)
		{
			(*tempapplistnum)++;
		}
		else if(retCode == RLT_EMV_PARTICALMATCH)
		{
			bSecond=1;
			(*tempapplistnum)++;
			continue;
		}
		else if(retCode == RLT_EMV_NOTMATCH)
		{
			bSecond=1;
			continue;
		}
		else if(retCode == RLT_ERR_EMV_CardBlock)
		{
		    if(bSecond == 0)
		    {
			    emvbase_free(temptermapp);
				return RLT_ERR_EMV_CardBlock;
		    }
		}
		else if(retCode == RLT_ERR_EMV_APPBLOCK)
		{
			bSecond=1;
			continue;

		}

		AppNumIndex++;
		bSecond=0;
	}
	emvbase_free(temptermapp);
    return RLT_EMV_OK;
}


unsigned char EMVBase_ChooseTermApp(EMVBase_TermAidlistUnionStruct *temp_UnionStruct)
{
    unsigned char i, j, temp;
    unsigned char *tempappnum;
    unsigned char noprioritynum = 0;
    unsigned char *tmpapprtiority = NULL;
    unsigned char *AppPriority = NULL;
    EMVBASE_LISTAPPDATA *tempAppAppData;
	EMVBASE_LISTAPPDATA *tempList=NULL;

    tempappnum = &(temp_UnionStruct->AppListCandidatenum);
    tempAppAppData = temp_UnionStruct->AppListCandidate;


    if(*tempappnum < 1)
    {
        return RLT_ERR_EMV_NoAppSel;
    }

	tmpapprtiority = emvbase_malloc(temp_UnionStruct->AppListCandidateMaxNum);
	AppPriority = emvbase_malloc(temp_UnionStruct->AppListCandidateMaxNum);
	tempList = emvbase_malloc(temp_UnionStruct->AppListCandidateMaxNum*sizeof(EMVBASE_LISTAPPDATA));

	if(tmpapprtiority==NULL || AppPriority==NULL || tempList==NULL)
	{
		if(tmpapprtiority) emvbase_free(tmpapprtiority);
		if(AppPriority) emvbase_free(AppPriority);
		if(tempList) emvbase_free(tempList);
		return RLT_EMV_ERR;
	}

	EMVBase_Trace("EMVBase_ChooseTermApp before\r\n");
	for(i=0; i<*tempappnum; i++)
	{
		EMVBase_TraceHex("aid", tempAppAppData[i].AID, tempAppAppData[i].AIDLen);
	}

    for(i = 0; i < *tempappnum; i++)
    {
        AppPriority[i] = (tempAppAppData + i)->Priority & 0x0F;
    }

    if(*tempappnum == 1)
    {
    }
    else
    {
        for(j = 0; j < *tempappnum - 1; j++)
        {
            for(i = 0; i < *tempappnum - 1 - j; i++)
            {
                if(AppPriority[i] > AppPriority[i + 1])
                {
                    temp = AppPriority[i];
                    AppPriority[i] = AppPriority[i + 1];
                    AppPriority[i + 1] = temp;

					memset(tempList, 0, sizeof(EMVBASE_LISTAPPDATA));
					memcpy(&tempList[0], &tempAppAppData[i], sizeof(EMVBASE_LISTAPPDATA));
					memcpy(&tempAppAppData[i], &tempAppAppData[i+1], sizeof(EMVBASE_LISTAPPDATA));
					memcpy(&tempAppAppData[i+1], &tempList[0], sizeof(EMVBASE_LISTAPPDATA));
                }
            }
        }

        noprioritynum = 0;

        for(i = 0; i < *tempappnum; i++)
        {
            if(AppPriority[i] == 0) { noprioritynum++; }
        }

        if(noprioritynum)              //have no priority app
        {
            memcpy(tmpapprtiority, &AppPriority[noprioritynum], *tempappnum - noprioritynum);
            memcpy(&tmpapprtiority[*tempappnum - noprioritynum], AppPriority, noprioritynum);
            memcpy(AppPriority, tmpapprtiority, *tempappnum);

			memcpy(tempList, &tempAppAppData[noprioritynum], (*tempappnum - noprioritynum)*sizeof(EMVBASE_LISTAPPDATA));
			memcpy(&tempList[*tempappnum - noprioritynum], tempAppAppData, noprioritynum*sizeof(EMVBASE_LISTAPPDATA));
			memcpy(tempAppAppData, tempList, (*tempappnum)*sizeof(EMVBASE_LISTAPPDATA));
        }
    }

	EMVBase_Trace("EMVBase_ChooseTermApp after\r\n");
	for(i=0; i<*tempappnum; i++)
	{
		EMVBase_TraceHex("aid", tempAppAppData[i].AID, tempAppAppData[i].AIDLen);
	}
	if(tmpapprtiority) emvbase_free(tmpapprtiority);
	if(AppPriority) emvbase_free(AppPriority);
	if(tempList) emvbase_free(tempList);

	return RLT_EMV_OK;
}

unsigned char EMVBase_FinalReadSelectRetData(EMVBASE_SELECT_RET* selectRet, unsigned char* DataOut, unsigned short LenOut, EMVBase_UnionStruct *tempEMVBase_UnionStruct)
{
    unsigned char k;
    unsigned short index, indexFCI, indexFCIProp, len, lenFCI, lenFCIProp, templen;
    unsigned short indexIssuerDiscret;
    unsigned char ret;
    unsigned char bIntable;
    EMVBASETAGCVLITEM *emvitem;
    unsigned char *temp;


    index = 0;

    if(DataOut[index] != 0x6F)          //FCI
    {
    	EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
        return RLT_ERR_EMV_IccDataFormat;         //FCI template
    }
    index++;

    if(EMVBase_ParseExtLen(DataOut, &index, &lenFCI))
    {
    	EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
        return RLT_ERR_EMV_IccDataFormat;
    }

	emvbase_avl_createsettagvalue(EMVTAG_FCITemplate, DataOut + index, lenFCI);

    indexFCI = index;

    while(index < indexFCI + lenFCI)
    {
        if(index >= LenOut)
		{
			EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
			return RLT_ERR_EMV_IccDataFormat;
		}

        if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }
        else if(DataOut[index] == 0x84)         //DF
        {
            if(selectRet->DFNameExist == 1)
			{
				EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
				return RLT_ERR_EMV_IccDataFormat;
			}
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &templen))
            {
            	EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
                return RLT_ERR_EMV_IccDataFormat;
            }

            selectRet->DFNameLen = templen;

            if((selectRet->DFNameLen > 16) || (selectRet->DFNameLen < 5))
			{
				EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
				return RLT_ERR_EMV_IccDataFormat;
			}
            memcpy(selectRet->DFName, DataOut + index, selectRet->DFNameLen);
            selectRet->DFNameExist = 1;

            index += templen;

            emvbase_avl_createsettagvalue(EMVTAG_DFName, selectRet->DFName, selectRet->DFNameLen);
        }
        else if(DataOut[index] == 0xA5)         //FCI
        {
            selectRet->FCIPropExist = 1;
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &lenFCIProp))
            {
            	EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
                return RLT_ERR_EMV_IccDataFormat;
            }
            indexFCIProp = index;

            while(index < indexFCIProp + lenFCIProp)
            {
        		if(DataOut[index] == 0xFF || DataOut[index] == 0x00) { index++; continue; }

				if(!memcmp((unsigned char*)&DataOut[index], "\x9F\x38", 2)) // PDOL
				{
					index += 2;

					if(EMVBase_ParseExtLen(DataOut, &index, &templen))
		            {
		            	EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					emvbase_avl_createsettagvalue(EMVTAG_PDOL, DataOut + index, templen);

		            index += templen;
				}
				else
		        {
		            k = DataOut[index];
		            if((k & 0x1F) == 0x1F)
		            {
		                index++;
		            }
		            index++;

		            if(EMVBase_ParseExtLen(DataOut, &index, &templen))
		            {
		            	EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
		                return RLT_ERR_EMV_IccDataFormat;
		            }
		            index += templen;
		        }
            }

            if(index != indexFCIProp + lenFCIProp)
            {
            	EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
                return RLT_ERR_EMV_IccDataFormat;
            }
        }
        else
        {
            k = DataOut[index];
            if((k & 0x1F) == 0x1F)
            {
                index++;
            }
            index++;

            if(EMVBase_ParseExtLen(DataOut, &index, &len))
            {
            	EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
                return RLT_ERR_EMV_IccDataFormat;
            }
            index += len;
        }
    }

    if(index != indexFCI + lenFCI)
    {
    	EMVBase_Trace("EMVBase_FinalReadSelectRetData line: %d\r\n", __LINE__);
        return RLT_ERR_EMV_IccDataFormat;
    }
    return RLT_EMV_OK;
}

unsigned char EMVBase_FinalSelectRetData(EMVBASE_APDU_RESP *apdu_r, EMVBase_UnionStruct *tempEMVBase_UnionStruct)
{
    EMVBASE_SELECT_RET selectRet;
    EMVBASE_LISTAPPDATA *tempselectedapp;
    EMVBASE_LISTAPPDATA *tempappdata;
    unsigned char tempselectedappno;
    unsigned char *tempappnum;
	unsigned char SW[2] = {0};


	SW[0] = apdu_r->SW1;
	SW[1] = apdu_r->SW2;
	emvbase_avl_createsettagvalue("\xD3", SW, 2);


    tempappnum = &(tempEMVBase_UnionStruct->EntryPoint->AppListCandidatenum);
    tempselectedapp = tempEMVBase_UnionStruct->EntryPoint->SelectedApp;
    tempappdata = tempEMVBase_UnionStruct->EntryPoint->AppListCandidate;
    tempselectedappno = tempEMVBase_UnionStruct->SelectedAppNo;

    if(apdu_r->SW1 == 0x90 && apdu_r->SW2 == 0x00) //Select OK
    {
        memset((unsigned char*)&selectRet, 0, sizeof(EMVBASE_SELECT_RET));

        if(EMVBase_FinalReadSelectRetData(&selectRet, apdu_r->DataOut, apdu_r->LenOut, tempEMVBase_UnionStruct) != RLT_EMV_OK)
        {
            EMVBase_Trace("EMVBase_FinalSelectRetData111 %d\r\n", *tempappnum);
            return RLT_EMV_APPSELECTTRYAGAIN;

        }
        EMVBase_Trace("EMVBase_FinalSelectRetData222\r\n");

        if(selectRet.DFNameExist == 0 || selectRet.FCIPropExist == 0)
        {
            if(*tempappnum > 1)
            {
                return RLT_EMV_APPSELECTTRYAGAIN;
            }
            else
            {
                EMVBase_Trace( "EMVBase_FinalSelectRetData333\r\n");
                return RLT_ERR_EMV_IccDataFormat;
            }
        }

        EMVBase_Trace("EMVBase_FinalSelectRetData444 %d\r\n", tempselectedappno);
		EMVBase_TraceHex("tempaid", (tempappdata + tempselectedappno)->AID, (tempappdata + tempselectedappno)->AIDLen);

        memcpy(tempselectedapp->AID, (tempappdata + tempselectedappno)->AID, (tempappdata + tempselectedappno)->AIDLen);
        tempselectedapp->AIDLen = (tempappdata + tempselectedappno)->AIDLen;

		EMVBase_Trace( "EMVBase_FinalSelectRetData777\r\n");
    }
    else    //current app selected fail,delete it from app list and select again.
    {
    	EMVBase_Trace( "select apdu not 9000!\r\n");
        if(*tempappnum > 1)
        {
            return RLT_EMV_APPSELECTTRYAGAIN;
        }
        else
        {
            return RLT_ERR_EMV_SWITCHINTERFACE;
        }
    }

    return RLT_EMV_OK;
}

unsigned char EMVBase_FinalSelect(EMVBase_UnionStruct *tempEMVBase_UnionStruct)
{
    unsigned char retCode;
    EMVBASE_APDU_SEND apdu_s;
    EMVBASE_APDU_RESP apdu_r;
    unsigned char tempselectappno;
    EMVBASE_LISTAPPDATA *tempAppAppData;


    tempAppAppData = tempEMVBase_UnionStruct->EntryPoint->AppListCandidate;
    tempselectappno = tempEMVBase_UnionStruct->SelectedAppNo;
    EMVBase_COMMAND_SELECT((tempAppAppData + tempselectappno)->AID, (tempAppAppData + tempselectappno)->AIDLen, 0, &apdu_s);
    tempEMVBase_UnionStruct->IsoCommand(&apdu_s, &apdu_r);

    if(apdu_r.ReadCardDataOk != RLT_EMV_OK)
    {
    	EMVBase_Trace( "SELECT APDU ERROR!\r\n");
        return RLT_ERR_EMV_APDUTIMEOUT;
    }

    retCode = EMVBase_FinalSelectRetData(&apdu_r, tempEMVBase_UnionStruct);
    EMVBase_Trace( "EMVBase_FinalSelectRetData = %d\r\n", retCode);
	if(retCode != RLT_EMV_OK)
	{
		return RLT_EMV_APPSELECTTRYAGAIN;
	}

    return retCode;
}

unsigned char EMVBase_FinalSelectedApp(EMVBase_UnionStruct *tempEMVBase_UnionStruct)
{
    unsigned char retCode;


    retCode = EMVBase_FinalSelect(tempEMVBase_UnionStruct);
    EMVBase_Trace( "EMVBase_FinalSelect = %d\r\n", retCode);

	if(retCode == RLT_EMV_OK || retCode == RLT_ERR_EMV_IccCommand || retCode == RLT_ERR_EMV_APDUTIMEOUT || retCode == RLT_ERR_EMV_IccDataFormat || retCode == RLT_ERR_EMV_SWITCHINTERFACE)
    {
        return retCode;
    }

    return RLT_EMV_APPSELECTTRYAGAIN;
}

unsigned char EMVBase_PDOLProcess(EMVBase_UnionStruct *tempEMVBase_UnionStruct, EMVBASE_APDU_SEND *apdu_s)
{
    unsigned char *pdolData;
    unsigned short index, indexOut;
    unsigned char k, m, *buf, bInTable;
    unsigned char len;
    EMVBASETAGCVLITEM *item;
    unsigned char *PDOL;
    unsigned char tag[4], taglen;
    EMVBASETAGCVLITEM *pdoldataitem;
    unsigned short templen = 0, tempPDOLlen = 0;
	unsigned char PDOLbexist = 0;
	unsigned char PDOLErrorFlag = 0;
	unsigned char tempselectappno;
    EMVBASE_LISTAPPDATA *tempAppAppData;
	unsigned char bVisa = 0;
	unsigned char TTQExist = 0;


    tempAppAppData = tempEMVBase_UnionStruct->EntryPoint->AppListCandidate;
    tempselectappno = tempEMVBase_UnionStruct->SelectedAppNo;
	if(0 == memcmp((tempAppAppData + tempselectappno)->AID, "\xA0\x00\x00\x00\x03\x10\x10", 7))
	{
		EMVBase_Trace("VISA AID!!\r\n");
		bVisa = 1;
	}

    item = emvbase_avl_gettagitempointer(EMVTAG_PDOL);

	if(item != NULL)
    {
        if(item->len)
        {
            PDOLbexist = 1;
        }
    }

	if(1 == PDOLbexist)
	{
	    pdolData = (unsigned char *)emvbase_malloc(255);
	    buf = (unsigned char *)emvbase_malloc(255);
	    PDOL = (unsigned char *)emvbase_malloc(300);
	    memset(pdolData, 0, 255);

		EMVBase_TraceHex("EMVBase_PDOLProcess", item->data, item->len);

	    index = 0;
	    indexOut = 0;

	    memset(PDOL, 0, 300);

	    if(item->len > 255)
	    {
	        tempPDOLlen = 255;
	    }
	    else
	    {
	        tempPDOLlen = item->len;
	    }
	    memcpy(PDOL, item->data, tempPDOLlen);
	}

    index = 0;

    while(index < tempPDOLlen)  //Process PDOL
    {
        if(PDOL[index] == 0xFF || PDOL[index] == 0x00) {index++; continue; }
        memset(buf, 0, 255);
        bInTable = 0;

        memset(tag, 0, sizeof(tag));
        taglen = 0;
        tag[taglen++] = PDOL[index];

        if((tag[0] & 0x1f) == 0x1f)
        {
            tag[taglen++] = PDOL[index + 1];

            if(tag[1] & 0x80)
            {
                tag[taglen++] = PDOL[index + 2];
            }
        }

		if(!memcmp(tag, "\x9F\x66", 2))
        {
            TTQExist = 1;
        }

		//if((memcmp(tag, "\x9F\x50", 2) < 0) || (memcmp(tag, "\x9F\x7F", 2) > 0) || (!memcmp(tag, "\x9F\x7A", 2)))
		{
        	pdoldataitem = emvbase_avl_gettagitempointer(tag);

        	if(pdoldataitem != NULL)
	        {
	            index += taglen;

				if(0x6F == tag[0])
				{
					templen = PDOL[index];
					index++;
				}
				else
				{
		            if(EMVBase_ParseExtLen(PDOL, &index, &templen))
		            {
		                emvbase_free(pdolData);
						emvbase_free(buf);
						emvbase_free(PDOL);
						#if 0 // 20210629
		                //return RLT_ERR_EMV_IccDataFormat;
						#else
						PDOLErrorFlag = 1;
						break;
						#endif
		            }

					if(index > tempPDOLlen) // 20210205
					{
		                emvbase_free(pdolData);
						emvbase_free(buf);
						emvbase_free(PDOL);
		                #if 0 // 20210629
		                //return RLT_ERR_EMV_IccDataFormat;
						#else
						PDOLErrorFlag = 1;
						break;
						#endif
		            }
				}

	            k = templen;
	            m = pdoldataitem->len;
	            if(pdoldataitem->datafomat & EMVTAGFORMAT_N)	//numeric
	            {
	                if(k >= m)
	                {
	                    if(m)
	                    {
	                        memcpy(&buf[k - m], pdoldataitem->data, m);
	                    }
	                    memcpy(&pdolData[indexOut], buf, k);
	                }
	                else
	                {
	                    if(m)
	                    {
	                        memcpy(buf, pdoldataitem->data, m);
	                    }
	                    memcpy(&pdolData[indexOut], &buf[m - k], k);
	                }
	            }
	            else if(pdoldataitem->datafomat & EMVTAGFORMAT_CN)	//compact numeric
	            {
	                if(m)
	                {
	                    memset(buf, 0xFF, 255);
	                    memcpy(buf, pdoldataitem->data, m);
	                    memcpy(&pdolData[indexOut], buf, k);
	                }
	                else
	                {
	                    memset(buf, 0x00, 255);
	                    memcpy(&pdolData[indexOut], buf, k);
	                }
	            }
	            else	//other formats
	            {
	                if(m)
	                {
	                    memcpy(buf, pdoldataitem->data, m);
	                }
	                memcpy(&pdolData[indexOut], buf, k);
	            }
	            indexOut += k;
	            bInTable = 1;
	        }
		}

        if(!bInTable)
        {
            index += taglen;

			if(0x6F == tag[0])
			{
				templen = PDOL[index];
				index++;
			}
			else
			{
	            if(EMVBase_ParseExtLen(PDOL, &index, &templen))
	            {
	                emvbase_free(pdolData);
					emvbase_free(buf);
					emvbase_free(PDOL);
	                #if 0 // 20210629
	                //return RLT_ERR_EMV_IccDataFormat;
					#else
					PDOLErrorFlag = 1;
					break;
					#endif
	            }

				if(index > tempPDOLlen) // 20210205
				{
	                emvbase_free(pdolData);
					emvbase_free(buf);
					emvbase_free(PDOL);
	                #if 0 // 20210629
	                //return RLT_ERR_EMV_IccDataFormat;
					#else
					PDOLErrorFlag = 1;
					break;
					#endif
	            }
			}

            k = templen;
            memcpy(&pdolData[indexOut], buf, k);
            indexOut += k;
        }
    }

	if(bVisa && ((0 == PDOLbexist) || (0 == TTQExist)))
	{
		return RLT_ERR_EMV_IccDataFormat;
	}

	if((PDOLbexist == 0) || (1 == PDOLErrorFlag)) // 20210629
    {
        memcpy(apdu_s->Command, "\x80\xA8\x00\x00", 4);
        apdu_s->Lc = 2;
        memcpy(apdu_s->DataIn, "\x83\x00", 2);
        apdu_s->Le = 256;
        apdu_s->EnableCancel = 1;
        return RLT_EMV_OK;
    }

    emvbase_avl_createsettagvalue(EMVTAG_PDOLData, pdolData, indexOut);
	EMVBase_TraceHex("EMVTAG_PDOLData: ", pdolData, indexOut);

    memcpy(apdu_s->Command, "\x80\xA8\x00\x00", 4);
    apdu_s->Lc = indexOut + 2;
    len = 0;
    apdu_s->DataIn[len++] = 0x83;

    if((indexOut & 0x80) > 0)                            //�������ֽ�
    {
        (apdu_s->Lc)++;
        apdu_s->DataIn[len++] = 0x81;
        apdu_s->DataIn[len++] = indexOut;
    }
    else
    {
        apdu_s->DataIn[len++] = indexOut;         //ֻ��һ���ֽ�
    }
    memcpy((unsigned char*)&apdu_s->DataIn[len], pdolData, indexOut);
    apdu_s->Le = 256;

    apdu_s->EnableCancel = 0;        //����ȡ��

    emvbase_free(pdolData);
    emvbase_free(buf);
    emvbase_free(PDOL);

    return RLT_EMV_OK;
}

unsigned char EMVBase_GPORetData(EMVBase_UnionStruct *tempEMVBase_UnionStruct, EMVBASE_APDU_RESP *apdu_r)
{
    unsigned short index, indexTemp, lenTemp, templen;
    unsigned char k;
    unsigned char bInTable;
    unsigned char ret;
	EMVBASETAGCVLITEM *item;
	unsigned short indexE4 = 0, lenE4 = 0;
	unsigned char buff[50] = {0};


	emvbase_avl_deletetag("\x91");
	emvbase_avl_deletetag("\x71");
	emvbase_avl_deletetag("\x72");

    if(apdu_r->SW1 == 0x90 && apdu_r->SW2 == 0x00)		//Get Processing Options OK
    {
        index = 0;

        if(apdu_r->DataOut[index] == 0x77)      //TLV coded data
        {
            index++;

            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &lenTemp))
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
            indexTemp = index;

            while(index < indexTemp + lenTemp)
            {
                if(index >= apdu_r->LenOut)
                {
                    return RLT_ERR_EMV_IccDataFormat;
                }

                if(apdu_r->DataOut[index] == 0xFF || apdu_r->DataOut[index] == 0x00) { index++; continue; }

				if(apdu_r->DataOut[index] == 0xD4) // Outcome data
		        {
		            index++;

		            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					emvbase_avl_createsettagvalue("\xD4", apdu_r->DataOut + index, templen);

		            index += templen;
		        }
				else if(apdu_r->DataOut[index] == 0x91) // Online Response Data 1
		        {
		            index++;

		            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					emvbase_avl_createsettagvalue(EMVTAG_IssuerAuthenData, apdu_r->DataOut + index, templen);

		            index += templen;
		        }
				else if(apdu_r->DataOut[index] == 0x71) // Online Response Data 2
		        {
		            index++;

		            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					emvbase_avl_createsettagvalue(EMVTAG_SCRIPT1, apdu_r->DataOut + index, templen);

		            index += templen;
		        }
				else if(apdu_r->DataOut[index] == 0x72) // Online Response Data 3
		        {
		            index++;

		            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					emvbase_avl_createsettagvalue(EMVTAG_SCRIPT2, apdu_r->DataOut + index, templen);

		            index += templen;
		        }
				else if(apdu_r->DataOut[index] == 0xD5) // UI Request on Outcome
		        {
		            index++;

		            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					emvbase_avl_createsettagvalue("\xD5", apdu_r->DataOut + index, templen);

		            index += templen;
		        }
				else if(apdu_r->DataOut[index] == 0xD6) // UI Request on Restart
		        {
		            index++;

		            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					emvbase_avl_createsettagvalue("\xD6", apdu_r->DataOut + index, templen);

		            index += templen;
		        }
				else if(apdu_r->DataOut[index] == 0xE1) // Data Record Present
		        {
		            index++;

		            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					emvbase_avl_createsettagvalue("\xE1", apdu_r->DataOut + index, templen);

		            index += templen;
		        }
				else if(apdu_r->DataOut[index] == 0xE2) // Discretionary Data Present
		        {
		            index++;

		            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					emvbase_avl_createsettagvalue("\xE2", apdu_r->DataOut + index, templen);

		            index += templen;
		        }
				else if(!memcmp((unsigned char*)&apdu_r->DataOut[index], "\xDF\x81\x16", 3)) // C-2  User Interface Request Data
				{
					index += 3;

					if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					memset(buff, 0x00, sizeof(buff));
					if(emvbase_avl_gettagvalue_spec("\xDF\x81\x16", buff, 0, 22))
					{
						emvbase_avl_createsettagvalue("\xDF\x81\x16", apdu_r->DataOut + index, templen);
					}
					else // second
					{
						memcpy(buff+22, apdu_r->DataOut + index, templen);
						emvbase_avl_createsettagvalue("\xDF\x81\x16", buff, 22+templen);
					}

		            index += templen;
				}
				else if(apdu_r->DataOut[index] == 0xE4) // C-2 E4
		        {
		            index++;

		            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }

					indexE4 = index;
					lenE4 = templen;

					while(index < indexE4 + lenE4)
					{
						if(index >= apdu_r->LenOut)
		                {
		                    return RLT_ERR_EMV_IccDataFormat;
		                }

						if(!memcmp((unsigned char*)&apdu_r->DataOut[index], "\xDF\x81\x29", 3)) // C-2  Outcome Parameter Set
						{
							index += 3;

							if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
				            {
				                return RLT_ERR_EMV_IccDataFormat;
				            }

							emvbase_avl_createsettagvalue("\xDF\x81\x29", apdu_r->DataOut + index, templen);

				            index += templen;
						}
						else if(!memcmp((unsigned char*)&apdu_r->DataOut[index], "\xDF\x81\x16", 3)) // C-2  User Interface Request Data
						{
							index += 3;

							if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
				            {
				                return RLT_ERR_EMV_IccDataFormat;
				            }

							memset(buff, 0x00, sizeof(buff));
							if(emvbase_avl_gettagvalue_spec("\xDF\x81\x16", buff, 0, 22))
							{
								emvbase_avl_createsettagvalue("\xDF\x81\x16", apdu_r->DataOut + index, templen);
							}
							else // second
							{
								memcpy(buff+22, apdu_r->DataOut + index, templen);
								emvbase_avl_createsettagvalue("\xDF\x81\x16", buff, 22+templen);
							}

				            index += templen;
						}
						else if(!memcmp((unsigned char*)&apdu_r->DataOut[index], "\xFF\x81\x05", 3)) // C-2  Data Record
						{
							index += 3;

							if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
				            {
				                return RLT_ERR_EMV_IccDataFormat;
				            }

							emvbase_avl_createsettagvalue("\xFF\x81\x05", apdu_r->DataOut + index, templen);

				            index += templen;
						}
						else if(!memcmp((unsigned char*)&apdu_r->DataOut[index], "\xFF\x81\x06", 3)) // C-2  Discretionary Data
						{
							index += 3;

							if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
				            {
				                return RLT_ERR_EMV_IccDataFormat;
				            }

							emvbase_avl_createsettagvalue("\xFF\x81\x06", apdu_r->DataOut + index, templen);

				            index += templen;
						}
						else
				        {
				            k = apdu_r->DataOut[index];
				            if((k & 0x1F) == 0x1F)
				            {
				                index++;
				            }
				            index++;

				            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
				            {
				                return RLT_ERR_EMV_IccDataFormat;
				            }
				            index += templen;
				        }

					}
		        }
				else
		        {
		            k = apdu_r->DataOut[index];
		            if((k & 0x1F) == 0x1F)
		            {
		                index++;
		            }
		            index++;

		            if(EMVBase_ParseExtLen(apdu_r->DataOut, &index, &templen))
		            {
		                return RLT_ERR_EMV_IccDataFormat;
		            }
		            index += templen;
		        }
            }

            if(index != indexTemp + lenTemp)
            {
                return RLT_ERR_EMV_IccDataFormat;
            }

			#if 0
			item = emvbase_avl_gettagitempointer(EMVTAG_AFL);
            if((item->len % 4) != 0)
            {
                return RLT_ERR_EMV_IccDataFormat;
            }
			#endif
        }
        else{ return RLT_ERR_EMV_IccDataFormat; }
    }
    else
	{
        return RLT_EMV_APPSELECTTRYAGAIN;
	}

    return RLT_EMV_OK;
}

unsigned char EMVBase_InitialApp(EMVBase_UnionStruct *tempEMVBase_UnionStruct)
{
    unsigned char retCode;
    EMVBASE_APDU_SEND apdu_s;
    EMVBASE_APDU_RESP apdu_r;
	int i = 0;


	emvbase_avl_deletetag("\xD4");
	emvbase_avl_deletetag("\xD5");
	emvbase_avl_deletetag("\xD6");
	emvbase_avl_deletetag("\xE1");
	emvbase_avl_deletetag("\xE2");
	emvbase_avl_deletetag("\xDF\x81\x16");
	emvbase_avl_deletetag("\xDF\x81\x29");
	emvbase_avl_deletetag("\xFF\x81\x05");
	emvbase_avl_deletetag("\xFF\x81\x06");

	retCode = EMVBase_PDOLProcess(tempEMVBase_UnionStruct, &apdu_s);
	EMVBase_Trace( "EMVBase_PDOLProcess retCode=%d\r\n", retCode);

	if(retCode != RLT_EMV_OK)
	{
	    if(RLT_ERR_EMV_IccDataFormat == retCode)
    	{
			return RLT_EMV_APPSELECTTRYAGAIN;
		}
		return retCode;
	}

	tempEMVBase_UnionStruct->IsoCommand(&apdu_s, &apdu_r);

	if(apdu_r.ReadCardDataOk != RLT_EMV_OK)
	{
		return RLT_ERR_EMV_APDUTIMEOUT;
	}

	retCode = EMVBase_GPORetData(tempEMVBase_UnionStruct, &apdu_r);
	EMVBase_Trace( "EMVBase_GPORetData retCode=%d\r\n", retCode);

	if(retCode != RLT_EMV_OK)
	{
	    if(RLT_ERR_EMV_IccDataFormat == retCode)
    	{
			return RLT_EMV_APPSELECTTRYAGAIN;
		}
		return retCode;
	}

	return retCode;
}

