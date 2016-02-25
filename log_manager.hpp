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
#include "caf/all.hpp"
#include "caf/io/all.hpp"
using std::string;
using std::map;
using std::thread;
using std::cin;
using std::cout;
using std::endl;

using OkAtom = caf::atom_constant<caf::atom("ok")>;
using FailAtom = caf::atom_constant<caf::atom("fail")>;
using AppendAtom = caf::atom_constant<caf::atom("append")>;
using BeginAtom = caf::atom_constant<caf::atom("begin")>;
using CommitAtom = caf::atom_constant<caf::atom("commit")>;
using AbortAtom = caf::atom_constant<caf::atom("abort")>;
using WriteAtom = caf::atom_constant<caf::atom("write")>;
using DataAtom = caf::atom_constant<caf::atom("data")>;
using CheckpointAtom = caf::atom_constant<caf::atom("checkpoint")>;

using UInt64 = unsigned long long;
const long long kMaxLogSize = 128 * 1024 * 1024;
const string kLogFileName = "claims.trans.log.";
const string kLogDataPrefix = "<$@$claims.data.prefix@$@>\n";
const string kLogDataSuffix = "<$@$claims.data.suffix@$@>\n";

class LogManager {
 public:
  static LogManager & getInstance() {
     static LogManager logManager;
     return logManager;
  }
 void setPath(string path){
   log_path = path;
 }
 void Startup(){
   pthread_t pid;
   pthread_create(&pid, NULL, MainThread, this);
 }
 void LogBegin(UInt64 Tid) {
   caf::scoped_actor self;
  // self->send(collector,BeginAtom::value,Tid);

   self->sync_send(collector,BeginAtom::value,Tid).await(
       [&](OkAtom) {}
   );
 }
 void LogWrite(UInt64 Tid, UInt64 Partid, UInt64 Pos, UInt64 Offset) {
   caf::scoped_actor self;
   self->sync_send(collector,WriteAtom::value,Tid,Partid,Pos,Offset).await(
       [&](OkAtom) {}
   );
 }
 void LogCommit(UInt64 Tid) {
   caf::scoped_actor self;
   self->sync_send(collector,CommitAtom::value,Tid).await(
       [&](OkAtom) {}
   );
 }
 void LogAbort(UInt64 Tid) {
   caf::scoped_actor self;
   self->sync_send(collector,AbortAtom::value,Tid).await(
       [&](OkAtom) {}
   );
 }
 void LogAppend(string  log){
   caf::scoped_actor self;
   self->sync_send(collector,AppendAtom::value,log).await(
       [&](OkAtom) {}
   );
 }
 void LogData(UInt64 Tid, UInt64 Partid, UInt64 Pos, UInt64 Offset,
              char * buffer, UInt64 len){
   caf::scoped_actor self;
   self->sync_send(collector,DataAtom::value,Tid, Partid, Pos,Offset,(UInt64)buffer, len).await(
        [&](OkAtom) {}
    );
 }
 private:
 LogManager(){ }
  string log_path = ".";
  FILE * log_head = NULL;
  UInt64 size  = (UInt64)0;
  caf::actor collector;
  caf::actor cleaner;

  static void * MainThread(void * arg){
    LogManager* logm =  (LogManager*)arg;
    logm->collector = caf::spawn(CollectorBehav, logm);
    logm->cleaner = caf::spawn(CleanerBehav, logm);
    caf::await_all_actors_done();
  }
  void Append(const string & log);
  void Append(const string & prefix, char * buffer, UInt64 len, const string & suffix);
  static void CollectorBehav(caf::event_based_actor * self, LogManager * logm);
  static void CleanerBehav(caf::event_based_actor * self, LogManager * logm);
};




#endif //  LOG_MANAGER_HPP_ 
