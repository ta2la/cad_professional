//
// Copyright (C) 2023 Petr Talla. [petr.talla@gmail.com]
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//		      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=============================================================================
#pragma once

#include "T2lObjectDisplable.h"
#include <T2lGObject.h>
#include <T2lPoint2.h>
#include <T2lPoint2Col.h>
#include "T2lActiveFile.h"
#include "T2lColor.h"
#include "T2lDefs.h"

#include <string>

namespace T2l
{

class StoredItem;
class Canvas; //TODO remove
class EntityLine;

//===================================================================
class CadObject_arc : public ObjectDisplable {
//===================================================================
public:
//<CONSTRUCTION>
    CadObject_arc( const Point2Col<double>& points, GFile* parent, int gid = 0, const char* style = "solid");
    virtual ~CadObject_arc(void);
//<METHODS>
    TATTR_OBJECT(Color,        color)
    TATTR_SIMPLE(double,       width)
    TATTR_OBJECT(std::string,  style)

    void displayGid_(EntityList& list, RefCol* scene);

    static void normalize(Point2F& p0, Point2F& p1, Point2F& p2);
    static bool isCcWise(const Point2F& center, const Point2F& dirPt, const Vector2F& dir);
    static bool centerRadius(const Point2F& p0, const Point2F& p1, const Point2F& p2, Point2F& center, double& radius);
    static void pointsCurve( Point2FCol& points, Point2F& p0, Point2F& p1, Point2F& p2 );
//===================================================================
//<OVERRIDES>
    virtual void             display(EntityList& list, RefCol* scene);
    virtual bool             loadFromStored(StoredItem* item, GFile* file);
    virtual void             saveToStored(StoredItem& item, GFile* file);
    virtual std::string      print();
    virtual ObjectDisplable* clone();
    virtual EIdentified      identifiedByPoint(const T2l::Canvas& canvas, const Point2F& pt);
    virtual int              snapCount() { return snapRawCount()+1; }
    virtual Point2F          snapGet(int index);
protected:
//<DATA>
    std::string symbolBeg_;
    std::string symbolEnd_;
};

} //namespace T2l
