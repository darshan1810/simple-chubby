
#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <cstdint>
#include <map>

#include <server/chubby_client.h>
#include "include/server.hh"
#include "include/event.hh"

typedef uint64_t FileHandlerId;

const FileHandlerId FAIL = 0;

const uint64_t READ = 0x1;
const uint64_t WRITE = 0x2;
const uint64_t CREATE_DIRECTORY = 0x4;
const uint64_t CREATE_FILE = 0x8;

const uint64_t EPHEMERAL = 0x10;

const uint64_t EV_LOCK_CHANGED = 0x100;
const uint64_t EV_CONTENT_MODIFIED = 0x200;
typedef std::function<void(std::string)> EventCallback;

enum ClientError {
  /* Bad arguments for the API call. */
  BAD_ARG,
  /* Operation fails in the filesystem. */
  FS_FAIL
};

/*
 * For server and/or library errors that must delivered to the client
 */
class ClientException : public std::exception {
public:
  ClientException(enum ClientError err) { errcode = err; }
  virtual ~ClientException() { }
  ClientError code() const { return errcode; }
  const char *what() {
    switch (errcode) {
      case BAD_ARG:
        return "BAD ARGUMENT";
      case FS_FAIL:
        return "FAILS IN FILESYSTEM";
      default:
        return "UNKNOWN ERROR CODE";
    }
  }
private:
  enum ClientError errcode;
};


class Client {
public:
  Client();
  ~Client();
  /*
   * Connect to a server
   */
  void open(const std::string &host_file);
  /*
   * Disconnect from a server
   */
  void close();
  /*
   * Test if still connected to a server
   *
   * Return Value: true if connected
   */
  bool isConnected();

  int increment(int input);
  int decrement(int input);

  /* Chubby APIs */
  FileHandlerId fileOpen(const std::string &file_name, Mode mode);
  void fileClose(FileHandlerId fdId);
  bool fileDelete(FileHandlerId fdId);
  bool getContentsAndStat(FileHandlerId fdId,
                          FileContent *file_content,
                          MetaData *meta_data);
  bool setContents(FileHandlerId fdId, const FileContent &file_content);
  void acquire(FileHandlerId fdId);
  bool tryAcquire(FileHandlerId fdId);
  void release(FileHandlerId fdId);

  /* Chubby event callbacks */
  void registerCallback(ChubbyEvent e, EventCallback cb);
  void deleteCallback(ChubbyEvent e);

private:
  xdr::chubby_client_handler<api_v1> *client;
  std::map<uint64_t, FileHandler> fdList;
  FileHandlerId nextFdId; /* FileHandlerId of the next  */

  void printFdList();
};

#endif /* __CLIENT_H__ */

