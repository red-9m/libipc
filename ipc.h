#ifndef _LIB_IPC_H_
#define _LIB_IPC_H_

#define IPC_NOFREE  -1 // No free channels available
#define IPC_OPENERR -2 // Error open channel
#define IPC_DSCERR  -3 // Not valid descriptor
#define IPC_OBJERR  -4 // Incorrect object size
#define IPC_TYPEERR -5 // Incorrect channel type

enum ipc_type
{
    tpMessage = 1,
    tpObject = 2
};

enum ipc_operation
{
    opBlock = 1,
    opNonblock = 2
};

/** @brief Create new listening channel and open it for read
 *
 *  Only read is available for connected channel
 *
 *  @param chName Channel name
 *  @param chType Channel type (message or object)
 *  @param chBlock Channel blocking mode for read/write operation (block or nonblock)
 *  @return non-negative - channel descriptor; negative - error
 */
extern int ipc_create_channel(const char* chName, enum ipc_type chType, enum ipc_operation chBlock);

/** @brief Connects to existing channel
 *
 *  Channel must be created with ipc_create_channel() on another side.
 *  Only write is available for connected channel
 *
 *  @param chName Channel name
 *  @param chType Channel type (must be same as passed to ipc_create_channel())
 *  @param chBlock Channel blocking mode for read/write operation (block or nonblock)
 *  @return non-negative - channel descriptor; negative - error
 */
extern int ipc_connect_channel(const char* chName, enum ipc_type chType, enum ipc_operation chBlock);

/** @brief Close channel by descriptor
 *
 *  @param chId Channel descriptor
 *  @return zero - channel closed; negative - error
 */
extern int ipc_close_channel(int chId);

/** @brief Write message to the channel and exit immediately
 *
 *  @param chId Channel descriptor
 *  @param msgText Text message
 *  @return non-negative - number of bytes written; negative - error
 */
extern int ipc_write_message(int chId, const char* msgText);

/** @brief Read message from the channel. Returns only when got a message
 *
 *  @param chId Channel descriptor
 *  @return non-negative - non-null - message; null - error
 */
extern const char* ipc_read_message(int chId);

/** @brief Write object to the channel and exit immediately
 *
 *  @param chId Channel descriptor
 *  @param obj Object to be written
 *  @param objSize Size of object
 *  @return non-negative - number of bytes written; negative - error
 */
extern int ipc_write_object(int chId, const void* obj, int objSize);

/** @brief Read object from the channel. Returns only when got a message
 *
 *  @param chId Channel descriptor
 *  @param obj Object to read to
 *  @param objSize Size of object
 *  @return non-negative - ok; negative - error
 */
extern int ipc_read_object(int chId, void* obj, int objSize);

#endif
