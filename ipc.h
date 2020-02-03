#ifndef _LIB_IPC_H_
#define _LIB_IPC_H_


// === IPC error codes =========================================================

/** No free channels are available */
#define IPC_NOFREE  -101

/** Not valid descriptor */
#define IPC_DSCERR  -102

/** Incorrect object size */
#define IPC_OBJERR  -103

/** Incorrect channel type */
#define IPC_TYPEERR -104

/** Incorrect or closed socket */
#define IPC_SOCKERR -105


// === Public types ============================================================

/**
 *  Channel transport type
 */
enum ipc_transport
{
    ipcFifo = 1,  /**< Unix Named Pipes: Unidirectional transport, many client could send to one server */
    ipcSock = 2   /**< Unix Domain Sockets: Bidirectional transport, one client at a time to one server */
};

/**
 *  Channel message type
 */
enum ipc_msg
{
    ipcFree    = 0,  /**< No message type, free channel */
    ipcMessage = 1,  /**< Channel exchanges with null terminated messages */
    ipcObject  = 2   /**< Channel exchanges with objects/structures */
};

/**
 *  Channel blocking mode
 */
enum ipc_block
{
    ipcBlock    = 1, /**< Synchronous read/write to channel */
    ipcNonblock = 2  /**< Asynchronous read/write to channel */
};


// === Public API ==============================================================

/** @brief  Create new listening channel
 *
 *  @param  chName         Channel name
 *  @param  transportType  Channel transport
 *  @param  msgType        Channel message type (message or object)
 *  @param  blockMode      Channel blocking mode for read/write operation
 *
 *  @return                non-negative - channel descriptor; negative - IPC error code or negative errno
 */
extern int ipc_create_channel(const char* chName, enum ipc_transport transportType,  enum ipc_msg msgType, enum ipc_block blockMode);

/** @brief  Connects to existing channel
 *
 *  Channel must be created with ipc_create_channel() on another side.
 *
 *  @param  chName         Channel name
 *  @param  transportType  Channel transport
 *  @param  msgType        Channel message type (must be same as passed to ipc_create_channel())
 *  @param  blockMode      Channel blocking mode for read/write operation
 *
 *  @return                non-negative - channel descriptor; negative - IPC error code or negative errno
 */
extern int ipc_connect_channel(const char* chName, enum ipc_transport transportType, enum ipc_msg msgType, enum ipc_block blockMode);

/** @brief  Close created or connected channel
 *
 *  @param  chId           Channel descriptor from ipc_create_channel() or ipc_connect_channel()
 *
 *  @return                zero - channel closed; negative - IPC error code
 */
extern int ipc_close_channel(int chId);

/** @brief  Write message to the channel
 *
 *  @param  chId           Channel descriptor
 *  @param  msgText        Text message
 *
 *  @return                non-negative - number of bytes written; negative - IPC error code or negative errno
 */
extern int ipc_write_message(int chId, const char* msgText);

/** @brief  Read message from the channel
 *
 *  @param  chId           Channel descriptor
 *
 *  @return                non-null - message; null - no message for non-blocking operation or error
 */
extern const char* ipc_read_message(int chId);

/** @brief  Write object to the channel
 *
 *  @param  chId           Channel descriptor
 *  @param  obj            Pointer to object to be written
 *  @param  objSize        Size of object
 *
 *  @return                non-negative - number of bytes written; negative - IPC error code or negative errno
 */
extern int ipc_write_object(int chId, const void* obj, int objSize);

/** @brief  Read object from the channel
 *
 *  @param  chId           Channel descriptor
 *  @param  obj            Pointer to object to read to
 *  @param  objSize        Size of object
 *
 *  @return                positive - ok; negative - IPC error code or negative errno
 */
extern int ipc_read_object(int chId, void* obj, int objSize);


#endif
