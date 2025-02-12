#include "macro.h"
#include <iostream>
#include <cstdlib>

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

using namespace std;

Macro::Macro(const string& userType, const string& userContent) : type(userType), content(userContent) {

    if (type == "keystroke") {
        callback = bind(&Macro::keystrokeCallback, this);
    } else if (type == "program") {
        callback = bind(&Macro::programCallback, this);
    } else {
        callback = nullptr;
    }

}

Macro::~Macro() {

}

void Macro::keystrokeCallback() {

}

void Macro::programCallback() {
    cout << "Executing program: " << content << endl;
    system(content.c_str());
}

void Macro::setType(const string& newType)  {
    type = newType;

    if (type == "keystroke") {
        callback = bind(&Macro::keystrokeCallback, this);
    } else if (type == "program") {
        callback = bind(&Macro::programCallback, this);
    } else {
        callback = nullptr;
    }
}

void Macro::setContent(const string& newContent) {
    content = newContent;
}

string Macro::getType() {
    return type;
}

string Macro::getContent() {
    return content;
}

void Macro::runCallback() {
    if (callback) {
        callback();
    } else {
        cout << "No callback set!" << endl;
    }
}



