//
// Copyright (C) 2022 Petr Talla. [petr.talla@gmail.com]
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
class CadObject_papper : public ObjectDisplable {
//===================================================================
public:
//<CONSTRUCTION>
    CadObject_papper(    const Point2F position, double factor, double drawingUnitInMm,
                         double papperWidthInMm, double papperHeightInMm, GFile* parent,
                         bool maskLess = false );
    ~CadObject_papper(   void );
//<METHODS>
    double               factor() { return factor_; }
    double               drawingUnitInMm()  { return drawingUnitInMm_;  }
    double               papperWidthInMm()  { return papperWidthInMm_;  }
    double               papperHeightInMm() { return papperHeightInMm_; }
    double               foldWidthInMm()    { return foldWidthInMm_;    }

    Box2F                papperBox();
//===================================================================
//<OVERRIDES>
    bool                 isOfType(FilterCadObject::ECadObjectType type) override { return type == FilterCadObject::ECO_PAPPER; }
    void                 display(EntityList& list, RefCol* scene) override;
    bool                 loadFromStored(StoredItem* item, GFile* file) override;
    void                 saveToStored(StoredItem& item, GFile* file) override;
    std::string          print() override;
    GObject::EIdentified identifiedByPoint(const Canvas& canvas, const Point2F& pt) override;
protected:
//<DATA>
    double               factor_;
    double               drawingUnitInMm_;
    double               papperWidthInMm_;
    double               papperHeightInMm_;
    double               foldWidthInMm_;

    bool                 maskLess_;
//<INTERNALS>
    static void display_line_(EntityList& list, double width, const Point2F& p0, const Point2F& p1,
                              const Color& color, unsigned char transp = 150, double ext = 0);
};

} //namespace T2l
