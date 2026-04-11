//
// Copyright (C) 2018 Petr Talla. [petr.talla@gmail.com]
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

namespace T2l
{

class StoredItem;
class Canvas; //TODO remove

//===================================================================
class CadObject_dimm : public ObjectDisplable {
//===================================================================
public:
//<CONSTRUCTION>
    CadObject_dimm( const Point2Col<double>& points, double dir, GFile* parent );
    ~CadObject_dimm(void);
//<METHODS>
    const Color& color() const { return color_; }
    void setColor(const Color& color) { color_ = color; }

    void setMetricText(bool value) { metricText_ = value; }

    double size() { return size_; }
    void sizeSet(double size) { size_ = size; }
//===================================================================
//<OVERRIDES>
    bool isOfType(FilterCadObject::ECadObjectType type) override { return type == FilterCadObject::ECO_DIMM; }
    virtual void display(EntityList& list, RefCol* scene);
    virtual bool loadFromStored(StoredItem* item, GFile* file);
    virtual void saveToStored(StoredItem& item, GFile* file);
    virtual std::string print();
    virtual ObjectDisplable* clone();

    virtual int      snapCount() { if (nondisplable()) return 0; return 4; }
    virtual Point2F  snapGet(int index);
protected:
    void calculateBasicGeometry_(Point2F& p0, Point2F& p1);
//<DATA>
    Color  color_;
    double size_;
    bool   metricText_;
    double dir_;


// <INTERNALS>
};

} //namespace T2l
