#ifndef			_CONCENTERAPP_H__
#define			_CONCENTERAPP_H__


/***** Defines *****/




/***** Type declarations *****/





/***** Variable declarations *****/





/***** Prototypes *****/
void ConcenterAppInit(void);

void ConcenterUploadStart(void);

void ConcenterUploadStop(void);

void ConcenterUploadPeriodSet(uint32_t period);

void ConcenterSensorDataSave(uint8_t *dataP, uint8_t length);

void ConcenterSensorDataUpload(void);

void ConcenterUploadEventSet(void);





#endif			// _CONCENTERAPP_H__
