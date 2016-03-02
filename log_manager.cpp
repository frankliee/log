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
#include "unistd.h"
using std::string;
using std::to_string;

LogManager LogService::LogM;
caf::actor LogService::Collector;
caf::actor LogService::Cleaner;

void LogService::CollectorBehav(caf::event_based_actor * self){
  cout << "log collect start" << endl;
  self->become(
      [=](AppendAtom, string & log)->caf::message {
          LogM.Append(log);
          return caf::make_message(OkAtom::value);
        },
      [=](BeginAtom, UInt64 id)->caf::message {
          LogM.Append(LogBeginContent(id));
          return caf::make_message(OkAtom::value);
        },
      [=](WriteAtom,UInt64 id, UInt64 partid, UInt64 pos,
          UInt64 offset)->caf::message {
          LogM.Append(LogWriteContent(id, partid, pos, offset));
          return caf::make_message(OkAtom::value);
        },
      [=](CommitAtom, UInt64 id)->caf::message{
          LogM.Append(LogCommitContent(id));
          return caf::make_message(OkAtom::value);
        },
      [=](AbortAtom, UInt64 id)->caf::message{
          LogM.Append(LogAbortContent(id));
          return caf::make_message(OkAtom::value);
        },
      [=](DataAtom, UInt64 Tid,UInt64 Partid, UInt64 pos, UInt64 offset,
          UInt64 buffer,UInt64 len)->caf::message{
          string prefix = "data<"+to_string(Tid)+","+to_string(Partid)+","+
              to_string(pos)+","+to_string(offset)+">\n"+kLogDataPrefix;
          LogM.Append(prefix,(char*)buffer,len,"\n"+kLogDataSuffix);
          return caf::make_message(OkAtom::value);
        },
      caf::others >> [=]() { cout << "unkown message" << endl; }
  );

}
void LogService::CleanerBehav(caf::event_based_actor * self){

}
void LogManager::Append(const string & log){
  //cout <<"log:" << log << endl;
  /* 新建日志文件 */
  if (log_head == nullptr) {
     struct timeval ts;
     gettimeofday(&ts, NULL);
     string file = log_path +"/"+ kLogFileName + to_string(ts.tv_sec);
     log_head = fopen(file.c_str(),"a");
     size = 0;
  }

  fputs(log.c_str(),log_head);
  size+=log.length();
  fflush(log_head);

  /* 日志文件已满 */

  if(size >= size_max ) {
    fclose(log_head);
    log_head = nullptr;
  }
}

void LogManager::Append(const string & prefix, char * buffer, UInt64 len, const string & suffix){
  /* 新建日志文件 */
  if (log_head == nullptr) {
     struct timeval ts;
     gettimeofday(&ts, NULL);
     string file = log_path +"/"+ kLogFileName +
         to_string(ts.tv_sec*1000*1000 + ts.tv_usec);
     log_head = fopen(file.c_str(),"a");
     size = 0;
  }

  fputs(prefix.c_str(),log_head);
  fwrite(buffer,sizeof(char),len,log_head);
  fputs(suffix.c_str(),log_head);
  size += prefix.length()+suffix.length()+len;
  fflush(log_head);

  /* 日志文件已满 */
  if(size >= size_max ) {

    fclose(log_head);
    log_head = nullptr;
  }
}


