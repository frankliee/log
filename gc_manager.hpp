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
 * /log/gc_manager.hpp
 *
 *  Created on: 2016年3月6日
 *      Author: imdb
 *		   Email: 
 * 
 * Description:
 *
 */

#ifndef GC_MANAGER_HPP_
#define GC_MANAGER_HPP_
#include <iostream>
#include <vector>
#include <thread>
#include <map>
#include <chrono>
#include <functional>
#include <unistd.h>
#include "caf/all.hpp"
#include "caf/io/all.hpp"
using std::map;
using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::pair;
using std::chrono::seconds;
using std::function;
using UInt64 = unsigned long long;
using GCAtom = caf::atom_constant<caf::atom("gc")>;
const int kDefaultGCTime = 1;
const int kDefaultGCBufferSize = 1;

class GCService {
 public:
  static vector<pair<int,UInt64>> GCBuffer;
  static map<int, function<void(UInt64)>> DeleteHandle;
  static caf::actor GC;
  static int GCTime;
  static int GCBufferSize;
  static int kInt;
  static int kDouble;
  static int kString;
  static void Collect( int type, void * ptr) {
    caf::scoped_actor self;
    self->delayed_send(GC, seconds(GCTime), GCAtom::value, type, (UInt64)ptr);
  }
  static void Startup() {
    GC = caf::spawn(GCBehav);
  }
  static void GCBehav(caf::event_based_actor * self);

};



#endif //  GC_MANAGER_HPP_ 
