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
 * /log/log.hpp
 *
 *  Created on: 2016年2月24日
 *      Author: imdb
 *		   Email: 
 * 
 * Description:
 *
 */

#ifndef LOG_MANAGER_HPP_
#define LOG_MANAGER_HPP_
#include <stdio.h>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <thread>
#include "unistd.h"
#include "caf/all.hpp"
#include "caf/io/all.hpp"
using std::string;
using std::map;
using std::thread;
using std::cin;
using std::cout;
using std::endl;
using std::to_string;

using OkAtom = caf::atom_constant<caf::atom("ok")>;
using FailAtom = caf::atom_constant<caf::atom("fail")>;
using AppendAtom = caf::atom_constant<caf::atom("append")>;
using BeginAtom = caf::atom_constant<caf::atom("begin")>;
using CommitAtom = caf::atom_constant<caf::atom("commit")>;
using AbortAtom = caf::atom_constant<caf::atom("abort")>;
using WriteAtom = caf::atom_constant<caf::atom("write")>;
using DataAtom = caf::atom_constant<caf::atom("data")>;
using CPAtom = caf::atom_constant<caf::atom("checkpoint")>;

using UInt64 = unsigned long long;
using UInt32 = unsigned int;
const long long kMaxLogSize = 128 * 1024 * 1024;
const string kLogFileName = "claims.trans.log.";
const string kLogDataPrefix = "<$@$claims.data.prefix@$@>\n";
const string kLogDataSuffix = "<$@$claims.data.suffix@$@>\n";

class LogManager {
 public:
  string log_path = ".";
  FILE * log_head = NULL;
  UInt64 size  = (UInt64)0;
  UInt64 size_max = kMaxLogSize;
  void Append(const string & log);
  void Append(const string & prefix, char * buffer, UInt64 len, const string & suffix);

};

class LogService {
 public:
  static LogManager LogM;
  static caf::actor Collector;
  static caf::actor Cleaner;
  inline static string LogBeginContent(UInt64 Tid){
    return "begin<"+to_string(Tid) +">\n";
  }

  inline static string LogWriteContent(UInt64 id,UInt64 partId, UInt64 pos,UInt64 offset) {
    return "write<"+to_string(id)+","+to_string(partId)+","+
        to_string(pos)+","+to_string(offset)+">\n";
  }

  inline static string LogCommitContent(UInt64 Tid) {
    return "commit<"+to_string(Tid)+">\n";
  }
  inline static string LogCommitContent(UInt64 Tid, UInt64 hdfsPos) {
    return "commit<"+to_string(Tid)+","+to_string(hdfsPos)+">\n";
  }
  inline static string LogAbortContent(UInt64 Tid) {
    return "abort<"+to_string(Tid)+">\n";
  }
  inline static string LogCPContent(UInt64 Tid,UInt64 partId,UInt64 memPos){
    return "checkpoint<"+to_string(Tid)+","+to_string(partId)
        +","+to_string(memPos)+">\n";
  }
  static void LogBegin(UInt64 Tid) {
     caf::scoped_actor self;
     self->sync_send(Collector,BeginAtom::value,Tid).await(
         [&](OkAtom) {}
     );
   }
  static void LogCP(UInt64 Tid, UInt64 partId, UInt64 memPos) {
    caf::scoped_actor self;
    self->sync_send(Collector,CPAtom::value,Tid,partId,memPos).await(
          [&](OkAtom) {}
    );
  }
  static void LogCommit(UInt64 Tid) {
    caf::scoped_actor self;
    self->sync_send(Collector,CommitAtom::value,Tid).await(
        [&](OkAtom) {}
    );
  }
  static void LogCommit(UInt64 Tid, UInt64 hdfsPos) {
    caf::scoped_actor self;
    self->sync_send(Collector,CommitAtom::value,Tid, hdfsPos).await(
        [&](OkAtom) {}
    );
  }
  static void LogAbort(UInt64 Tid) {
     caf::scoped_actor self;
     self->sync_send(Collector,AbortAtom::value,Tid).await(
         [&](OkAtom) {}
     );
   }
   static void LogAppend(string  log){
     caf::scoped_actor self;
     self->sync_send(Collector,AppendAtom::value,log).await(
         [&](OkAtom) {}
     );
   }
   static void LogData(UInt64 Tid, UInt64 Partid, UInt64 Pos, UInt64 Offset,
                char * buffer, UInt64 len){
     caf::scoped_actor self;
     self->sync_send(Collector,DataAtom::value,Tid, Partid, Pos,Offset,
                     (UInt64)buffer, len).await(
          [&](OkAtom) {}
      );
   }
   static void LogWrite(UInt64 Tid, UInt64 Partid, UInt64 Pos, UInt64 Offset) {
     caf::scoped_actor self;
     self->sync_send(Collector,WriteAtom::value,Tid,Partid,Pos,Offset).await(
         [&](OkAtom) {}
     );
   }
   static void Startup(){
     Collector = caf::spawn(CollectorBehav);
     Cleaner = caf::spawn(CleanerBehav);
    }

   static void CollectorBehav(caf::event_based_actor * self );
   static void CleanerBehav(caf::event_based_actor * self );
};


#endif //  LOG_MANAGER_HPP_ 
