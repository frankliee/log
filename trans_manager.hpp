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
 * /log/trans_manager.hpp
 *
 *  Created on: 2016年2月25日
 *      Author: imdb
 *		   Email: 
 * 
 * Description:
 *
 */

#ifndef TRANS_MANAGER_HPP_
#define TRANS_MANAGER_HPP_

#include <vector>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

const int kINGES = 1;
const int kREAD = 2;
const char kUVISIBLE = 0;
const char kVISIBLE = 1;

const int kLocalTrans = 1;
const int kRemoteTrans = 2;

using std::vector;
using std::string;

class Strip {
 public:
  UInt64 PartId;
  UInt64 Pos;
  UInt32 Offset;

};

class Trans{
 public:
  UInt64 TransId;
  int Type;
  char Visible;
  UInt32 NPart;
  UInt32 NCommit;
  UInt32 NAbort;
  vector<Strip*> StripList;
  Trans():TransId(0),Type(kREAD),Visible(kUVISIBLE),
      NPart(0),NCommit(0),NAbort(0){}
  ~Trans(){
    for(auto strip : StripList)
      delete strip;
  }
};

class TransManager{
 public:
  static TransManager & getInstance() {
    static TransManager transm;
    return transm;
  }
 private:
  TransManager(){}
  TransManager(const TransManager & transm) {};
  TransManager & operator  = (const TransManager & transm){}
};

class TransLocalService{
 public:
  TransLocalService() {}
};

class TransWebService{
 public:
  TransWebService() { }
 private:
  TransManager & tranm = TransManager::getInstance();

};



#endif //  TRANS_MANAGER_HPP_ 
