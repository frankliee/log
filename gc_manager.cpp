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
 * /log/gc_manager.cpp
 *
 *  Created on: 2016年3月6日
 *      Author: imdb
 *		   Email: 
 * 
 * Description:
 *
 */
#include "gc_manager.hpp"

int GCService::GCTime = kDefaultGCTime;
int GCService::GCBufferSize = kDefaultGCBufferSize;
vector<pair<int, UInt64>> GCService::GCBuffer;
caf::actor GCService::GC;
int GCService::kInt = 1;
int GCService::kDouble = 2;
int GCService::kString = 3;
map<int, function<void(UInt64)>> GCService::DeleteHandle = {
     {GCService::kInt, [](UInt64 ptr){ delete (int *)ptr;}},
     {GCService::kDouble, [](UInt64 ptr){ delete (double *)ptr;}},
     {GCService::kString, [](UInt64 ptr){ delete (string *)ptr;}}
};


void GCService::GCBehav(caf::event_based_actor * self) {
  self->become(
      [=](GCAtom,int type, UInt64 ptr) {
        GCBuffer.emplace_back(type, ptr);
        if (GCBuffer.size() >= GCBufferSize) {
           for (auto & garbage:GCBuffer) {
             auto type = garbage.first;
             auto ptr = garbage.second;
             DeleteHandle[type](ptr);
               }
           GCBuffer.clear();
          }
       }
  );
}
