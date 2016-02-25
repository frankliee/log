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
 * /log/main.cpp
 *
 *  Created on: 2016年2月24日
 *      Author: lizhif
 *		   Email: 885626704@qq.com
 * 
 * Description:
 *
 */
#include <iostream>
#include <thread>
#include <vector>
#include "unistd.h"
#include "sys/time.h"
#include "log_manager.hpp"
#include "caf/all.hpp"
#include "caf/io/all.hpp"
using std::cin;
using std::cout;
using std::endl;
using std::thread;
using std::vector;
auto logm = LogManager::getInstance();
void task1(int id) {
  for(auto p=0;p<10000;p++ ) {
    logm.LogBegin(id*10000 + p);
    logm.LogCommit(id*10000 + p);
  }
}
/*
int main (){
  logm.Startup();
  sleep(1);
  vector<thread> v;
  for(auto i= 0;i<100;i++)
    v.push_back(thread(task1,i));
  for(auto i = 0;i<100;i++)
      v[i].join();
}
*/
const int size =3000;
char buffer[size];

void task2(int id) {
  for(auto i=0;i<100000;i++) {
    logm.LogData(id,1,2,3,buffer,size);
    //logm.LogCommit(id);
    //logm.LogAbort(id);
  }
}
int main() {
   for(auto i=0;i<size-1;i++)
     buffer[i]='a';
   buffer[size-1]=0;
  logm.Startup();
  sleep(1);
  vector<thread> v;
  for(auto i=0;i<10;i++)
    v.push_back(thread(task2,i));
  for(auto i=0;i<10;i++)
    v[i].join();
}

