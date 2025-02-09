#include "macro.h"

/* class Macro:
  type: String
  content: String
  callback: Function

  Macro(userType, userContent):
    type = userType
    content = userContent
    if (type == “keystroke”):
      callback = keystrokeCallback
    else if (type == “program”):
      callback = programCallback
    else:
      callback = NULL

  void keystrokeCallback():
    // TODO: trigger keystroke

  void programCallback():
    // TODO: run executable

  // Other helper functions
  void setType()
  void setContent()
*/


Macro::Macro(const std::string& userType, const std::string& userContent) : type(userType), content(userContent) {

    if (type == "keystroke") {

        callback = std::bind(&Macro::keystrokeCallback, this);

    } else if (type == "program") {

        callback = std::bind(&Macro::programCallback, this);

    } else {

        callback = nullptr;

    }

}

Macro::~Macro() {

}

void keystrokeCallback() {

}

void programCallback() {

}

void setType() {

}

void setContent() {

}



