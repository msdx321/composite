#include <stdio.h>

#include <cos_component.h>
#include <cos_debug.h>
#include <cos_kernel_api.h>
#include <cos_types.h>
#include <sl.h>

#include "ostask.h"
#include <capmgr.h>
#include <memmgr.h>

#include <cFE_emu.h>

#include <cfe_error.h>

/* shared memory regions per thread.. */
static union shared_region *shared_regions[MAX_NUM_THREADS] = { NULL };

int       have_registered_sync_callback = 0;
asndcap_t sync_callback_delegates[CFE_TIME_MAX_NUM_SYNCH_FUNCS];

int32
sync_callback_handler()
{
	int       i;
	asndcap_t callback;

	for (i = 0; i < CFE_TIME_MAX_NUM_SYNCH_FUNCS; i++) {
		callback = sync_callback_delegates[i];

		if (callback) { cos_asnd(callback, 1); }
	}
	return CFE_SUCCESS;
}

int
emu_request_memory(spdid_t client, thdid_t tid)
{
	vaddr_t our_addr = 0;
	int     id       = memmgr_shared_page_allocn(SHARED_REGION_NUM_PAGES, &our_addr);
	int     cb_tmp   = (int)ps_faa((unsigned long *)&have_registered_sync_callback, 1);
	thdid_t thd = tid == 0 ? cos_thdid() : tid;

	assert(client == cos_inv_token());
	assert(id > 0 && our_addr);
	shared_regions[thd] = (void *)our_addr;

	/* FIXME: This is broken if applications can stop (because then the handler could get auto-deregistered) */
	if (!cb_tmp) CFE_TIME_RegisterSynchCallback(sync_callback_handler);

	return id;
}

arcvcap_t
emu_create_aep_thread(spdid_t client, thdclosure_index_t idx, cos_channelkey_t key)
{
	struct sl_thd *        thd;
	sched_param_t          aep_priority;
	struct cos_defcompinfo child_dci;
	arcvcap_t extrcv;

	cos_defcompinfo_childid_init(&child_dci, client);

	thd = sl_thd_aep_alloc_ext(&child_dci, NULL, idx, 1, 0, key, 0, 0, &extrcv);
	assert(thd);

	aep_priority = sched_param_pack(SCHEDP_PRIO, CFE_INVERT_PRIO - CFE_TIME_1HZ_TASK_PRIORITY);
	sl_thd_param_set(thd, aep_priority);

	return extrcv;
}

thdid_t
emu_create_thread(spdid_t client, thdclosure_index_t idx)
{
	struct sl_thd *        thd;
	sched_param_t          thd_priority;
	struct cos_defcompinfo child_dci;
	arcvcap_t extrcv;

	cos_defcompinfo_childid_init(&child_dci, client);

	thd = sl_thd_aep_alloc_ext(&child_dci, NULL, idx, 0, 0, 0, 0, 0, NULL);
	assert(thd);

	thd_priority = sched_param_pack(SCHEDP_PRIO, CFE_INVERT_PRIO - CFE_TIME_1HZ_TASK_PRIORITY);
	sl_thd_param_set(thd, thd_priority);

	return sl_thd_thdid(thd);
}

void
emu_stash_set_thdid(thdid_t tid)
{
	shared_regions[cos_thdid()]->cfe_es_createChildTask.tid = tid;
}

thdclosure_index_t
emu_stash_retrieve_thdclosure()
{
	return shared_regions[cos_thdid()]->cfe_es_createChildTask.idx;
}

spdid_t
emu_stash_retrieve_spdid()
{
	return cos_inv_token();
}

void
emu_sched_thd_block_timeout(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);
	s->sched_thd_block_timeout.result = sl_thd_block_timeout(s->sched_thd_block_timeout.deptid, s->sched_thd_block_timeout.abs_timeout);
}

/* Methods that wrap cFE methods
 * They use data in memory shared with the calling component
 */

int32
emu_CFE_ES_CalculateCRC(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_CalculateCRC(s->cfe_es_calculateCRC.Data, s->cfe_es_calculateCRC.DataLength,
	                           s->cfe_es_calculateCRC.InputCRC, s->cfe_es_calculateCRC.TypeCRC);
}

int32
emu_CFE_ES_CopyToCDS(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_CopyToCDS(s->cfe_es_copyToCDS.CDSHandle, s->cfe_es_copyToCDS.DataToCopy);
}

int32
emu_CFE_ES_CreateChildTask(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_CreateChildTask(&s->cfe_es_createChildTask.TaskId, s->cfe_es_createChildTask.TaskName,
	                              s->cfe_es_createChildTask.FunctionPtr, NULL, 0,
	                              s->cfe_es_createChildTask.Priority, s->cfe_es_createChildTask.Flags);
}

int32
emu_CFE_ES_GetAppID(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_GetAppID(&s->cfe_es_getAppID.AppId);
}

int32
emu_CFE_ES_GetAppName(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_GetAppName(s->cfe_es_getAppName.AppName, s->cfe_es_getAppName.AppId, s->cfe_es_getAppName.BufferLength);
}

int32
emu_CFE_ES_GetAppIDByName(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_GetAppIDByName(&s->cfe_es_getAppIDByName.AppId, s->cfe_es_getAppIDByName.AppName);
}

int32
emu_CFE_ES_GetAppInfo(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_GetAppInfo(&s->cfe_es_getAppInfo.AppInfo, s->cfe_es_getAppInfo.AppId);
}

int32
emu_CFE_ES_GetGenCount(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_GetGenCount(s->cfe_es_getGenCount.CounterId, &s->cfe_es_getGenCount.Count);
}

int32
emu_CFE_ES_GetGenCounterIDByName(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_GetGenCounterIDByName(&s->cfe_es_getGenCounterIDByName.CounterId,
	                                    s->cfe_es_getGenCounterIDByName.CounterName);
}

int32
emu_CFE_ES_GetResetType(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_GetResetType(&s->cfe_es_getResetType.ResetSubtype);
}

int32
emu_CFE_ES_GetTaskInfo(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_GetTaskInfo(&s->cfe_es_getTaskInfo.TaskInfo, s->cfe_es_getTaskInfo.TaskId);
}

int32
emu_CFE_ES_RegisterCDS(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_RegisterCDS(&s->cfe_es_registerCDS.CDS_Handle, s->cfe_es_registerCDS.BlockSize,
	                          s->cfe_es_registerCDS.Name);
}

int32
emu_CFE_ES_RestoreFromCDS(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_RestoreFromCDS(s->cfe_es_restoreFromCDS.RestoreToMemory, s->cfe_es_restoreFromCDS.CDSHandle);
}


int32
emu_CFE_ES_RunLoop(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_RunLoop(&s->cfe_es_runLoop.RunStatus);
}

int32
emu_CFE_ES_WriteToSysLog(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_ES_WriteToSysLog("%s", s->cfe_es_writeToSysLog.String);
}

int32
emu_CFE_EVS_Register(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_EVS_Register(s->cfe_evs_register.filters, s->cfe_evs_register.NumEventFilters,
	                        s->cfe_evs_register.FilterScheme);
}

int32
emu_CFE_EVS_SendEvent(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_EVS_SendEvent(s->cfe_evs_sendEvent.EventID, s->cfe_evs_sendEvent.EventType, "%s",
	                         s->cfe_evs_sendEvent.Msg);
}

int32
emu_CFE_FS_Decompress(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_FS_Decompress(s->cfe_fs_decompress.SourceFile, s->cfe_fs_decompress.DestinationFile);
}

int32
emu_CFE_FS_ReadHeader(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_FS_ReadHeader(&s->cfe_fs_writeHeader.Hdr, s->cfe_fs_writeHeader.FileDes);
}

int32
emu_CFE_FS_WriteHeader(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_FS_WriteHeader(s->cfe_fs_writeHeader.FileDes, &s->cfe_fs_writeHeader.Hdr);
}

int32
emu_CFE_SB_CreatePipe(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_CreatePipe(&s->cfe_sb_createPipe.PipeId, s->cfe_sb_createPipe.Depth,
	                         s->cfe_sb_createPipe.PipeName);
}

int32
emu_CFE_SB_DeletePipe(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_DeletePipe(s->cfe_sb_deletePipe.PipeId);
}

uint16
emu_CFE_SB_GetChecksum(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_GetChecksum((CFE_SB_MsgPtr_t)s->cfe_sb_msg.Msg);
}

void
emu_CFE_SB_GenerateChecksum(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	CFE_SB_GenerateChecksum((CFE_SB_MsgPtr_t)s->cfe_sb_msg.Msg);
}

uint16
emu_CFE_SB_GetCmdCode(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_GetCmdCode((CFE_SB_MsgPtr_t)s->cfe_sb_msg.Msg);
}

CFE_SB_MsgId_t
emu_CFE_SB_GetMsgId(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_GetMsgId((CFE_SB_MsgPtr_t)s->cfe_sb_msg.Msg);
}

void
emu_CFE_SB_SetMsgId(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	CFE_SB_SetMsgId((CFE_SB_MsgPtr_t)s->cfe_sb_setMsgId.Msg, s->cfe_sb_setMsgId.MsgId);
}

void
emu_CFE_SB_GetMsgTime(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];
	CFE_TIME_SysTime_t   time;

	assert(s);

	time = CFE_SB_GetMsgTime((CFE_SB_MsgPtr_t)&s->cfe_sb_msg.Msg);
	s->time                   = time;
}

uint16
emu_CFE_SB_GetTotalMsgLength(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_GetTotalMsgLength(&s->cfe_sb_msg.Msg);
}

void
emu_CFE_SB_SetTotalMsgLength(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	CFE_SB_SetTotalMsgLength((CFE_SB_MsgPtr_t)s->cfe_sb_setTotalMsgLength.Msg, s->cfe_sb_setTotalMsgLength.TotalLength);
}

uint16
emu_CFE_SB_GetUserDataLength(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_GetUserDataLength((CFE_SB_MsgPtr_t)s->cfe_sb_msg.Msg);
}

void
emu_CFE_SB_InitMsg(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	CFE_SB_InitMsg(s->cfe_sb_initMsg.MsgBuffer, s->cfe_sb_initMsg.MsgId, s->cfe_sb_initMsg.Length,
	               s->cfe_sb_initMsg.Clear);
}

int32
emu_CFE_SB_RcvMsg(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];
	CFE_SB_MsgPtr_t      BufPtr;
	int32                result;
	int                  len;

	assert(s);

	result = CFE_SB_RcvMsg(&BufPtr, s->cfe_sb_rcvMsg.PipeId, s->cfe_sb_rcvMsg.TimeOut);
	/* We want to save the message contents to the shared region
	 * But we need to be sure there is something to copy, so we check the call was successful
	 */
	if (result == CFE_SUCCESS) {
		len = CFE_SB_GetTotalMsgLength(BufPtr);
		if (len > EMU_BUF_SIZE) PRINTC("%s:%d %u %d\n", __func__, __LINE__, s->cfe_sb_rcvMsg.PipeId, len);
		//assert(len <= EMU_BUF_SIZE);
		len = len > EMU_BUF_SIZE ? EMU_BUF_SIZE : len;
		memcpy(s->cfe_sb_rcvMsg.Msg, (char *)BufPtr, len);
	}
	return result;
}

int32
emu_CFE_SB_SetCmdCode(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)s->cfe_sb_setCmdCode.Msg, s->cfe_sb_setCmdCode.CmdCode);
}

int32
emu_CFE_SB_SendMsg(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_SendMsg((CFE_SB_MsgPtr_t)s->cfe_sb_msg.Msg);
}

int32
emu_CFE_SB_SubscribeEx(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_SubscribeEx(s->cfe_sb_subscribeEx.MsgId, s->cfe_sb_subscribeEx.PipeId,
	                          s->cfe_sb_subscribeEx.Quality, s->cfe_sb_subscribeEx.MsgLim);
}

void
emu_CFE_SB_TimeStampMsg(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	CFE_SB_TimeStampMsg((CFE_SB_MsgPtr_t)s->cfe_sb_msg.Msg);
}

boolean
emu_CFE_SB_ValidateChecksum(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_SB_ValidateChecksum((CFE_SB_MsgPtr_t)s->cfe_sb_msg.Msg);
}

struct {
	size_t size;
	void * tbl_ptr; /* FIXME: Wrap CFE_TBL_ReleaseAddress to set this back to NULL */
} table_info[CFE_TBL_MAX_NUM_HANDLES];

int32
emu_CFE_TBL_GetAddress(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];
	CFE_TBL_Handle_t     handle;
	int32                result;

	assert(s);

	handle = s->cfe_tbl_getAddress.TblHandle;
	result = CFE_TBL_GetAddress(&table_info[handle].tbl_ptr, handle);

	if (result == CFE_SUCCESS || result == CFE_TBL_INFO_UPDATED) {
		memcpy(s->cfe_tbl_getAddress.Buffer, table_info[handle].tbl_ptr, table_info[handle].size);
	}

	return result;
}

int32
emu_CFE_TBL_GetInfo(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_TBL_GetInfo(&s->cfe_tbl_getInfo.TblInfo, s->cfe_tbl_getInfo.TblName);
}

int32
emu_CFE_TBL_Share(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_TBL_Share(&s->cfe_tbl_share.TblHandle, s->cfe_tbl_share.TblName);
}

int32
emu_CFE_TBL_Load(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return CFE_TBL_Load(s->cfe_tbl_load.TblHandle, s->cfe_tbl_load.SrcType, s->cfe_tbl_load.SrcData);
}

int32
emu_CFE_TBL_Modified(spdid_t client)
{
	/* FIXME: We assume the passed data is valid, which isn't safe */

	union shared_region *s = shared_regions[cos_thdid()];
	CFE_TBL_Handle_t     handle;
	void *               tbl_ptr;
	size_t               size;

	assert(s);

	handle  = s->cfe_tbl_modified.TblHandle;
	tbl_ptr = table_info[handle].tbl_ptr;
	size    = table_info[handle].size;

	assert(tbl_ptr);
	memcpy(tbl_ptr, s->cfe_tbl_modified.Buffer, size);

	return CFE_TBL_Modified(handle);
}


int32
emu_CFE_TBL_Register(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];
	int32 result;

	assert(s);

	result = CFE_TBL_Register(&s->cfe_tbl_register.TblHandle, s->cfe_tbl_register.Name,
                   s->cfe_tbl_register.TblSize, s->cfe_tbl_register.TblOptionFlags, NULL);

	if (result == CFE_SUCCESS || result == CFE_TBL_INFO_RECOVERED_TBL) {
		table_info[s->cfe_tbl_register.TblHandle].size = s->cfe_tbl_register.TblSize;
	}

	return result;
}

void
emu_CFE_TIME_Add(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	s->cfe_time_add.Result = CFE_TIME_Add(s->cfe_time_add.Time1, s->cfe_time_add.Time2);
}

void
emu_CFE_TIME_Compare(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	s->cfe_time_compare.Result = CFE_TIME_Compare(s->cfe_time_compare.Time1, s->cfe_time_compare.Time2);
}

void
emu_CFE_TIME_GetTime(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	s->time = CFE_TIME_GetTime();
}

void
emu_CFE_TIME_Print(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	CFE_TIME_Print(s->cfe_time_print.PrintBuffer, s->cfe_time_print.TimeToPrint);
}

int32
emu_CFE_TIME_RegisterSynchCallback(cos_channelkey_t key)
{
	int i;
	for (i = 0; i < CFE_TIME_MAX_NUM_SYNCH_FUNCS; i++) {
		if (!sync_callback_delegates[i]) {
			sync_callback_delegates[i] = capmgr_asnd_key_create(key);
			return CFE_SUCCESS;
		}
	}
	return CFE_TIME_TOO_MANY_SYNCH_CALLBACKS;
}

int32
emu_OS_cp(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_cp(s->os_cp.src, s->os_cp.dest);
}

int32
emu_OS_creat(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_creat(s->os_creat.path, s->os_creat.access);
}

int32
emu_OS_FDGetInfo(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_FDGetInfo(s->os_FDGetInfo.filedes, &s->os_FDGetInfo.fd_prop);
}

int32
emu_OS_fsBytesFree(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_fsBytesFree(s->os_fsBytesFree.name, &s->os_fsBytesFree.bytes_free);
}

int32
emu_OS_mkdir(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_mkdir(s->os_mkdir.path, s->os_mkdir.access);
}

int32
emu_OS_open(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_open(s->os_open.path, s->os_open.access, s->os_open.mode);
}

os_dirp_t
emu_OS_opendir(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_opendir(s->os_opendir.path);
}

int32
emu_OS_mv(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_mv(s->os_cp.src, s->os_cp.dest);
}

int32
emu_OS_read(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_read(s->os_read.filedes, s->os_read.buffer, s->os_read.nbytes);
}

void
emu_OS_readdir(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	s->os_readdir.dirent   = *OS_readdir(s->os_readdir.directory);
}

int32
emu_OS_remove(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_remove(s->os_remove.path);
}

int32
emu_OS_rename(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_rename(s->os_rename.old_filename, s->os_rename.new_filename);
}

int32
emu_OS_rmdir(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_rmdir(s->os_rmdir.path);
}

int32
emu_OS_stat(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_stat(s->os_stat.path, &s->os_stat.filestats);
}

int32
emu_OS_write(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_write(s->os_write.filedes, s->os_write.buffer, s->os_write.nbytes);
}

int32
emu_OS_BinSemCreate(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_BinSemCreate(&s->os_semCreate.sem_id, s->os_semCreate.sem_name, s->os_semCreate.sem_initial_value,
	                       s->os_semCreate.options);
}

int32
emu_OS_CountSemCreate(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_CountSemCreate(&s->os_semCreate.sem_id, s->os_semCreate.sem_name, s->os_semCreate.sem_initial_value,
	                         s->os_semCreate.options);
}

int32
emu_OS_MutSemCreate(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_MutSemCreate(&s->os_mutSemCreate.sem_id, s->os_mutSemCreate.sem_name, s->os_mutSemCreate.options);
}

int32
emu_OS_QueueCreate(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_QueueCreate(&s->os_queueCreate.queue_id, s->os_queueCreate.queue_name, s->os_queueCreate.queue_depth, s->os_queueCreate.data_size, s->os_queueCreate.flags);
}

int32
emu_OS_QueueGetIdByName(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_QueueGetIdByName(&s->os_queueGetIdByName.queue_id, s->os_queueGetIdByName.queue_name);
}

int32
emu_OS_QueueGet(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_QueueGet(s->os_queueGet.queue_id, (void *)(s->os_queueGet.buffer), s->os_queueGet.size, &s->os_queueGet.size_copied, s->os_queueGet.timeout);
}

int32
emu_OS_TaskGetIdByName(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_TaskGetIdByName(&s->os_taskGetIdByName.task_id, s->os_taskGetIdByName.task_name);
}

int32
emu_OS_SymbolLookup(spdid_t client)
{
	union shared_region *s = shared_regions[cos_thdid()];

	assert(s);

	return OS_SymbolLookup(&s->os_symbolLookup.symbol_address, s->os_symbolLookup.symbol_name);
}
