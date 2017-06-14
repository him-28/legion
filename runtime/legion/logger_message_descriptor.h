//
//  logger_message_descriptor.h
//  Legion
//
//  Created by Alan Heirich on 5/5/17.
//
//

#ifndef LoggerMessageDescriptor_h
#define LoggerMessageDescriptor_h

#include "realm/logging.h"

#include <stdio.h>
#include <stdlib.h>

#define REPORT_LEGION_ERROR(code, html, fmt, ...)               \
  {                                                             \
    Legion::Internal::MessageDescriptor descriptor(code, html); \
    char message[4096];                                         \
    snprintf(message, 4096, fmt, ##__VA_ARGS__);                \
    Legion::Internal::Runtime::report_error_message(            \
        descriptor, __FILE__, __LINE__, message);               \
  }

#define REPORT_LEGION_WARNING(code, html, fmt, ...)             \
  {                                                             \
    Legion::Internal::MessageDescriptor descriptor(code, html); \
    char message[4096];                                         \
    snprintf(message, 4096, fmt, ##__VA_ARGS__);                \
    Legion::Internal::Runtime::report_warning_message(          \
        descriptor, __FILE__, __LINE__, message);               \
  }

namespace Legion{
  namespace Internal {
    class MessageDescriptor {
    public:
      MessageDescriptor() : mID(0), mDescription(0) { }
      MessageDescriptor(Realm::LoggerMessageID id, 
                        std::string htmlDescription)
      {
        mID = id;
#ifdef LOG_MESSAGE_KEEP_DESCRIPTION
        mDescription = htmlDescription;
#endif
      }
      Realm::LoggerMessageID id() const{ return mID; }
      
      std::string description() const
      {
#ifdef LOG_MESSAGE_KEEP_DESCRIPTION
        return mDescription;
#else
        return "LoggerMessageDescriptor description not saved, "
          "recompile with LOG_MESSAGE_KEEP_DESCRIPTION to save it";
#endif
      }
      
      std::string formattedOutput() const 
      {
        char buffer[32];
        sprintf(buffer, "%d", (int)mID);
        return "{ " + std::string(buffer) + ", \"" + description() + "\" },";
      }
      
      std::string operator << (const std::string &t) 
      {
        char buffer[32];
        sprintf(buffer, "%d", (int)mID);
        return std::string(buffer) + t;
      }
                  
  private:
      Realm::LoggerMessageID mID;
      std::string mDescription;
    };
  }
}



#endif /* logger_message_descriptor.h */
