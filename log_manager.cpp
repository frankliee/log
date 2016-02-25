/*
 * Copyright [2012-2015] DaSE@ECNU
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * /log/log.cpp
 *
 *  Created on: 2016年2月24日
 *      Author: imdb
 *		   Email: 
 * 
 * Description:
 *
 */
#include <stdio.h>
#include <string>
#include "log_manager.hpp"
#include "time.h"
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "sys/time.h"
using std::string;
using std::to_string;

void LogManager::CollectorBehav(caf::event_based_actor * self, LogManager * logm){
  cout << "log collect start" << endl;
  self->become(
      [=](AppendAtom, string & log)->caf::message {
            cout << "logm:" << (UInt64)(&logm) << endl;
          logm->Append(log);
          return caf::make_message(OkAtom::value);
        },
      [=](BeginAtom, UInt64 Tid)->caf::message {
          logm->Append("begin<"+to_string(Tid) +">\n");
          return caf::make_message(OkAtom::value);
        },
      [=](WriteAtom,UInt64 Tid, UInt64 Partid, UInt64 pos,
          UInt64 offset)->caf::message {
          logm->Append("write<"+to_string(Tid)+","+to_string(Partid)+","+
                       to_string(pos)+","+to_string(offset)+">\n");
          return caf::make_message(OkAtom::value);
        },
      [=](CommitAtom, UInt64 Tid)->caf::message{
          logm->Append("commit<"+to_string(Tid)+">\n");
          return caf::make_message(OkAtom::value);
        },
      [=](AbortAtom, UInt64 Tid)->caf::message{
          logm->Append("abort<"+to_string(Tid)+">\n");
          return caf::make_message(OkAtom::value);
        },
      [=](DataAtom, UInt64 Tid,UInt64 Partid, UInt64 pos, UInt64 offset,
          UInt64 pointer,UInt64 len)->caf::message{
          return caf::make_message(OkAtom::value);
        },
      caf::others >> [=]() { cout << "unkown message" << endl; }
  );

}
void LogManager::CleanerBehav(caf::event_based_actor * self, LogManager * logm){

}
void LogManager::Append(const string & log){
  cout <<"log:" << log << endl;
  /* 新建日志文件 */
 // if(log_head == NULL) {
    //struct timeval ts;
  //  gettimeofday(&ts, NULL);

     cout <<"size: "<<size << endl;

  //}
  /*
  cout << log_head << endl;
  //fputs(log.c_str(),log_head);
  fprintf(log_head,"%s",log.c_str());
  cout << "finish write1" << endl;
  size+=log.length();
  cout << "finish write2" << endl;
  fclose(log_head);
  cout << "finish write3" << endl;
 */
  /* 日志文件已满 */
    /*
  if(size >= kMaxLogSize ) {
    fclose(log_head);
    log_head = nullptr;
  }
  */
}
