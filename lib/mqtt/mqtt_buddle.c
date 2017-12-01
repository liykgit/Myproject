/**
 * @file
 * @brief
 *
 * @version V0.0.1
 * @author zhongyh
 * @date
 * @note
 *   Ò»žöŽóµÄÏûÏ¢³Ø£¬ÓÃÓÚŽæ·Åž÷ÖÖŽýŽŠÀíµÄÏûÏ¢
 *   free -> tobe send -> tobe receive ack -> tobe handle callback -> free
 *
 *
 * @copy
 *
 * ŽËŽúÂëÎª ÉîÛÚœ­²šÁúµç×ÓÓÐÏÞ¹«ËŸÏîÄ¿ŽúÂë£¬ÈÎºÎÈËŒ°¹«ËŸÎŽŸ­Ðí¿É²»µÃžŽÖÆŽ«²¥£¬»òÓÃÓÚ
 * ±Ÿ¹«ËŸÒÔÍâµÄÏîÄ¿¡£±ŸËŸ±£ÁôÒ»ÇÐ×·Ÿ¿ÈšÀû¡£
 *
 * <h1><center>&copy; COPYRIGHT 2014 longsyswifi.com </center></h1>
 */


#include "mqtt.h"
#include "mqtt_buddle.h"
#include "mqtt_lib.h"
#include "common.h"

vg_mutex_t lock;
vg_sem_t g_sendbuddle_sem;
vg_sem_t g_flightbuddle_sem;
vg_sem_t g_pendbuddle_sem;


mqtt_package_t send_buddle[SEND_MAX];
mqtt_package_t flight_buddle[FLIGHT_MAX];
mqtt_package_t pend_buddle[PEND_MAX];

static int free_count = 0;
static int send_count = 0;
static int flight_count = 0;
static int pend_count = 0;

int get_send_count()
{
	return send_count;
}

int get_flight_count()
{
	return flight_count;
}

int get_pend_count()
{
	return pend_count;
}

void mqtt_buddle_init() {
	int i;

	memset(send_buddle, 0, sizeof(mqtt_package_t) * SEND_MAX);
	for(i=0; i<SEND_MAX; i++) {
		send_buddle[i].attr = ATTR_SEND;
	}

	memset(flight_buddle, 0, sizeof(mqtt_package_t) * FLIGHT_MAX);
	for(i=0; i<FLIGHT_MAX; i++) {
		flight_buddle[i].attr = ATTR_FLIGHT;
	}

	memset(pend_buddle, 0, sizeof(mqtt_package_t) * PEND_MAX);
	for(i=0; i<PEND_MAX; i++) {
		pend_buddle[i].attr = ATTR_PEND;
	}

	vg_create_mutex(&lock, "buddle_lock");

	free_count = BUDDLE_MAX;
	send_count = 0;
	flight_count = 0;
	pend_count = 0;

	vg_create_sem(&g_sendbuddle_sem, "send_lock");
	vg_create_sem(&g_flightbuddle_sem, "flight_lock");
	vg_create_sem(&g_pendbuddle_sem, "pend_lock");

}

/************************************************************************
 * @brief œ«Ò»žöÒÑŸ­È¡³öÀŽµÄbuddle item·Å»Øfree list
 * @param[in]
 * @return  0 :
 *         -1 :
 * @note OK
************************************************************************/
void FreeBuddle(mqtt_package_t *pItem) {
	unsigned char t_attr = pItem->attr;

	if(pItem->payload != NULL){
		vg_free(pItem->payload);
		pItem->payload == NULL;
	}

	memset(pItem, 0, sizeof(mqtt_package_t));
	pItem->attr = t_attr;
	switch(t_attr)
	{
	case ATTR_SEND:
		if(send_count >= SEND_MAX)
		{
			vg_release_sem(&g_sendbuddle_sem);
		}
		send_count--;
		break;
	case ATTR_FLIGHT:
		flight_count--;
		break;
	case ATTR_PEND:
		if(pend_count >= PEND_MAX)
		{
			vg_release_sem(&g_pendbuddle_sem);
		}
		pend_count--;
		break;
	}
	free_count++;

	return;
}


void ResetBuddle()
{
	int i;

	if(send_count >= SEND_MAX)
	{
		vg_release_sem(&g_sendbuddle_sem);
	}
	if(pend_count >= PEND_MAX)
	{
		vg_release_sem(&g_pendbuddle_sem);
	}

	vg_get_mutex(&lock);
	free_count = BUDDLE_MAX;
	send_count		= 0;
	flight_count		= 0;
	pend_count		= 0;

	for(i=0; i<SEND_MAX; i++) {
		memset(&send_buddle[i], 0, sizeof(mqtt_package_t));
		send_buddle[i].attr = ATTR_SEND;
	}

	for(i=0; i<FLIGHT_MAX; i++) {
		memset(&flight_buddle[i], 0, sizeof(mqtt_package_t));
		flight_buddle[i].attr = ATTR_FLIGHT;
	}

	for(i=0; i<PEND_MAX; i++) {
		memset(&pend_buddle[i], 0, sizeof(mqtt_package_t));
		pend_buddle[i].attr = ATTR_PEND;
	}
	vg_put_mutex(&lock);
}
/************************************************************************
 * @brief
 * @param[in]
 * @return  0 :
 *         -1 :
 * @note ŽÓbuddleÖÐÈ¡Ò»žöfreeÏî£¬ÓÃÓÚsend
 * @note
************************************************************************/
mqtt_package_t *GetFree2Send() {
	int i = 0;
	mqtt_package_t *pRet = NULL;

//	get_mutex(&lock);
	for(i=0; i<SEND_MAX; i++)
	{
		if(send_buddle[i].used == BUDDLE_FREE)
		{
			pRet = &send_buddle[i];
//			pRet->used = BUDDLE_USED;
			send_count++;
			free_count--;
//			printf("<%d> send %d, %d, %d, %d\n", vg_get_tick(), free_count, send_count, flight_count, pend_count);
//			put_mutex(&lock);
			return pRet;
		}
	}
//	put_mutex(&lock);

	return pRet;
}

/************************************************************************
 * @brief œ«Ò»žöÏûÏ¢·ÅÈësend list£¬ÓÉÓÚŽËÇ°È¡³öfreeÏîÊ±ÒÑŒÆËãÁËcount£¬ÕâÀïÎÞÐèµ÷Õû
 * @param[in]
 * @return  0 :
 *         -1 :
 * @note
************************************************************************/
/*void Put2Send(mqtt_package_t *pMsg) {
	get_mutex(lock);
	pMsg->next			= pt_Send;
	pt_Send			= pMsg;
	send_count ++;
	put_mutex(lock);

	print_usage("Put2Send");
}*/


/************************************************************************
 * @brief ŽÓSend listÖÐµÃµœ×îŸÉµÄÒ»žö
 * @param[in]
 * @return  0 :
 *         -1 :
 * @note ŽÓÁÐ±íÈ¡³öºóÐèœ«ŽËÏûÏ¢·¢ËÍ³öÈ¥£¬È»ºóœ«Ö®ÖÃÎªÏÂÒ»žö×ŽÌ¬
 * @note OK
************************************************************************/
mqtt_package_t * GetLastSendBuddle() {
	int i;
	mqtt_package_t *pRet = NULL;

	if(send_count >= SEND_MAX)
	{
		vg_release_sem(&g_sendbuddle_sem);
	}

//	get_mutex(&lock);
	for(i=0; i<SEND_MAX; i++)
	{
		if(send_buddle[i].used == BUDDLE_USED)
		{
			pRet = &send_buddle[i];
//			put_mutex(&lock);
			return pRet;
		}
	}

//	put_mutex(&lock);
	return pRet;
}

/************************************************************************
 * @brief œ«Ò»žöSENDÊôÐÔµÄÏûÏ¢·Åµœflight List
 * @param[in]
 * @return  0 :
 *         -1 :
 * @note
************************************************************************/
void MoveSend2Flight(mqtt_package_t *pItem)
{
	mqtt_package_t *pRet = NULL;

	//printf("MoveSend2Flight flight_count = %d\n", flight_count);
	if(flight_count >= FLIGHT_MAX) {
		//mqtt_printf("<ERR> flight_count is %d(max is %d)\n", flight_count, FLIGHT_MAX);
		return;
	}

	pRet = GetFree2Flight();
//	get_mutex(&lock);
	if(pRet)
	{
		memcpy(pRet, pItem, sizeof(mqtt_package_t));
		pRet->attr = ATTR_FLIGHT;
		pRet->used = BUDDLE_USED;
		//clean up the send buddle
		// pRet->payload is pointer to pItem->payload, so, don't free pItem->payload in FreeBuddle()
		pItem->payload = NULL;
		FreeBuddle(pItem);
	}
//	put_mutex(&lock);

}

void MoveFlight2Send(mqtt_package_t *pItem)
{
	mqtt_package_t *pRet = NULL;

	if(send_count >= SEND_MAX) {
		return;
	}

	pRet = GetFree2Send();
//	get_mutex(&lock);
	if(pRet){
		memcpy(pRet, pItem, sizeof(mqtt_package_t));
		pRet->attr = ATTR_SEND;
		pRet->used = BUDDLE_USED;

		// pRet->payload is pointer to pItem->payload, so, don't free pItem->payload in FreeBuddle()
		pItem->payload = NULL;
		FreeBuddle(pItem);
	}
//	put_mutex(&lock);
}

//////////////////////////////////////////////////////////////////////////
/************************************************************************
 * @brief
 * @param[in]
 * @return  0 :
 *         -1 :
 * @note ŽÓbuddleÖÐÈ¡Ò»žöfreeÏî£¬ÓÃÓÚsend
 * @note
************************************************************************/
mqtt_package_t *GetFree2Flight() {
	int i;
	mqtt_package_t *pRet = NULL;

	//printf("GetFree2Flight flight_count = %d\n", flight_count);
	if(flight_count >= FLIGHT_MAX) {
		//mqtt_printf("<ERR> flight_count is %d(max is %d)\n", flight_count, FLIGHT_MAX);
		return pRet;
	}

//	get_mutex(&lock);
	for(i=0; i<FLIGHT_MAX; i++)
	{
		if(flight_buddle[i].used == BUDDLE_FREE)
		{
			pRet = &flight_buddle[i];
			flight_count++;
			free_count--;
//			printf("<%d> flight %d, %d, %d, %d\n", vg_get_tick(), free_count, send_count, flight_count, pend_count);
//			put_mutex(&lock);
			return pRet;
		}
	}
//	put_mutex(&lock);

	return pRet;
}

/************************************************************************
 * @brief œ«Ò»žöÏûÏ¢·ÅÈësend list£¬ÓÉÓÚŽËÇ°È¡³öfreeÏîÊ±ÒÑŒÆËãÁËcount£¬ÕâÀïÎÞÐèµ÷Õû
 * @param[in]
 * @return  0 :
 *         -1 :
 * @note
************************************************************************/
/*void Put2Recv(mqtt_package_t *pMsg) {
	get_mutex(lock);
	pMsg->next			= pt_Flight;
	pt_Flight			= pMsg;
	flight_count ++;
	put_mutex(lock);
	print_usage("Put2Recv");
}

*/

/************************************************************************
 * @brief ŽÓrecv_listÖÐÕÒµœmsg_idµÄÏûÏ¢
 * @param[in]
 * @return  0 :
 *         -1 :
 * @note
************************************************************************/
mqtt_package_t *GetFromFlightByID(unsigned short msg_id) {
	int i;
	mqtt_package_t *pRet = NULL;

//	get_mutex(&lock);
	for(i=0; i<FLIGHT_MAX; i++)
	{
		if(flight_buddle[i].used == BUDDLE_USED &&
			flight_buddle[i].msg_id == msg_id)
		{
			pRet = &flight_buddle[i];
//			put_mutex(&lock);
			return pRet;
		}
	}

//	put_mutex(&lock);

	return pRet;
}

mqtt_package_t *GetFromFlightByType(int type) {
	int i;
	mqtt_package_t *pRet = NULL;

//	get_mutex(&lock);

	for(i=0; i<FLIGHT_MAX; i++)
	{
		if(flight_buddle[i].used == BUDDLE_USED &&
			flight_buddle[i].type == type)
		{
			pRet = &flight_buddle[i];
//			put_mutex(&lock);
			return pRet;
		}
	}

//	put_mutex(&lock);

	return pRet;
}


// void SendToPending(mqtt_package_t *pItem) {
// 	// lock
// 	pItem->attr			= ATTR_FREE;
// 	pItem->next			= pt_free;
// 	pt_free				= pItem;
// 	pending_count ++;
// 	// unlock
// }

/************************************************************************
 * @brief ŽÓpending listÖÐÈ¡³ö×îŸÉÒ»žöÔªËØ£¬ÏàÓŠµØpendingÊýÁ¿ŒõÒ»
 * @param[in]
 * @return  0 :
 *         -1 :
 * @note
************************************************************************/
mqtt_package_t *GetLastPendingBuddle() {
	int i;
	mqtt_package_t		*pRet = NULL;

	if(pend_count >= PEND_MAX)
	{
		vg_release_sem(&g_pendbuddle_sem);
	}

	//get_mutex(&lock);

	for(i=0; i<PEND_MAX; i++)
	{
		if(pend_buddle[i].used == BUDDLE_USED)
		{
			pRet = &pend_buddle[i];
			break;
		}
	}

	//put_mutex(&lock);

	return pRet;

}

/************************************************************************
 * @brief ŽÓFreeÖÐ·ÖÅäÒ»žö£¬·ÅÔÚ¹ÒÆðÁÐ±íÖÐ
 * @param[in]
 * @return  0 :
 *         -1 :
 * @note OK
************************************************************************/
mqtt_package_t *GetFree2Pending() {
	int i;
	mqtt_package_t *pRet = NULL;

	if(pend_count >= PEND_MAX) {
		//mqtt_printf("<ERR> pending_count = %d(max is %d)\n", pend_count, PEND_MAX);
		vg_wait_sem(&g_pendbuddle_sem, -1);
	}

	//get_mutex(&lock);
	for(i=0; i<PEND_MAX; i++)
	{
		if(pend_buddle[i].used == BUDDLE_FREE)
		{
			pRet = &pend_buddle[i];
//			pRet->used = BUDDLE_USED;
			pend_count++;
			free_count--;
//			printf("<%d> pend %d, %d, %d, %d\n", vg_get_tick(), free_count, send_count, flight_count, pend_count);
			break;
		}
	}
	//put_mutex(&lock);

	return pRet;
}


void MoveFlight2Pending(mqtt_package_t *pItem) {
	mqtt_package_t *pRet = NULL;

	if(pItem->attr != ATTR_FLIGHT) {
		return;
	}

	if(pend_count >= PEND_MAX) {
		vg_wait_sem(&g_pendbuddle_sem, -1);
	}

//	get_mutex(&lock);
	pRet = GetFree2Pending();
	if(pRet)
	{
		memcpy(pRet, pItem, sizeof(mqtt_package_t));
		pRet->attr = ATTR_PEND;
		pRet->used = BUDDLE_USED;
		//clean up the send buddle

		// pRet->payload is pointer to pItem->payload, so, don't free pItem->payload in FreeBuddle()
		pItem->payload = NULL;
		FreeBuddle(pItem);
	}

//	put_mutex(&lock);
}


/*
*	0 ok,	-1 err.
*/
int GetFlightCount()
{
	return flight_count;
}

/*
*	0 ok,	-1 err.
*/
int GetPendingCount()
{
	return pend_count;
}

void ClearSendBuddle()
{
	int i;
	mqtt_package_t *pRet = NULL;

	for(i=0; i<SEND_MAX; i++)
	{
		//printf("send_buddle[%d].used = %d\n", i, send_buddle[i].used);
		if(send_buddle[i].used == BUDDLE_USED)
		{
			pRet = &send_buddle[i];
			FreeBuddle(pRet);
		}
	}

	send_count = 0;

	for(i=0; i<FLIGHT_MAX; i++)
	{
		//printf("flight_buddle[%d].used = %d\n", i, flight_buddle[i].used);
		if(flight_buddle[i].used == BUDDLE_USED)
		{
			pRet = &flight_buddle[i];
			FreeBuddle(pRet);
		}
	}
	flight_count = 0;

	//printf("ClearSendBuddle flight_count = %d\n", flight_count);
}
void RemoveTimeoutBuddle()
{
	int i;
	mqtt_package_t		*pRet = NULL;

//	get_mutex(&lock);
	//remove timeout flight msg
	for(i=0; i<FLIGHT_MAX; i++)
	{
		if(flight_buddle[i].used == BUDDLE_USED)
		{
			pRet = &flight_buddle[i];
			if((vg_get_tick() - pRet->tick) > (mqtt.keepalive))
			{
				FreeBuddle(pRet);
			}
		}
	}

	//remove timeout send msg
	for(i=0; i<PEND_MAX; i++)
	{
//		printf("send_buddle[%d].used = %d\n", i, send_buddle[i].used);
		if(pend_buddle[i].used == BUDDLE_USED)
		{
			pRet = &pend_buddle[i];
			if((vg_get_tick() - pRet->tick) > (mqtt.keepalive))
			{
				FreeBuddle(pRet);
			}
		}
	}

//	put_mutex(&lock);
}

mqtt_package_t *check_flight(int ping_times)
{
	int i;
	mqtt_package_t *pRet = NULL;

	for(i=0; i<FLIGHT_MAX; i++){
		if(flight_buddle[i].used == BUDDLE_USED){
			pRet = &flight_buddle[i];
			if((vg_get_tick() - pRet->tick) > mqtt.keepalive){
				if(pRet->times + ping_times > MQTT_RETRY_TIMES){
					return pRet;
				}else{
					// timeout resend
					pRet->dup = 1;
					MoveFlight2Send(pRet);
				}
			}
		}
	}
	return NULL;
}
