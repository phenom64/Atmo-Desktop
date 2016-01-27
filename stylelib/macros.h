#ifndef MACROS_H
#define MACROS_H

#define SUNKEN state & (QStyle::State_Sunken | QStyle::State_Selected | QStyle::State_On)
#define HOVER state & QStyle::State_MouseOver
#define ENABLED state & QStyle::State_Enabled

#define castOpt(_Type_, _varName_, _fromVar_) const QStyleOption##_Type_ *_varName_ = qstyleoption_cast<const QStyleOption##_Type_ *>(_fromVar_)
#define castObj(_Type_, _varName_, _fromVar_) _Type_ _varName_ = qobject_cast<_Type_>(_fromVar_)

#define sAdjusted(_X1_, _Y1_, _X2_, _Y2_) adjusted(bool(sides&Left)*_X1_, bool(sides&Top)*_Y1_, bool(sides&Right)*_X2_, bool(sides&Bottom)*_Y2_)
#define sAdjust(_X1_, _Y1_, _X2_, _Y2_) adjust(bool(sides&Left)*_X1_, bool(sides&Top)*_Y1_, bool(sides&Right)*_X2_, bool(sides&Bottom)*_Y2_)
#define sShrink(_S_) sAdjust(_S_, _S_, -_S_, -_S_)
#define sShrinked(_S_) sAdjusted(_S_, _S_, -_S_, -_S_)
#define shrinked(_S_) adjusted(_S_, _S_, -_S_, -_S_)
#define shrink(_S_) adjust(_S_, _S_, -_S_, -_S_)
#define sGrow(_S_) sAdjust(-_S_, -_S_, _S_, _S_)
#define sGrowed(_S_) sAdjusted(-_S_, -_S_, _S_, _S_)
#define growed(_S_) adjusted(-_S_, -_S_, _S_, _S_)
#define grow(_S_) adjust(-_S_, -_S_, _S_, _S_)

#define ISURLBTN inherits("KDEPrivate::KUrlNavigatorButton")

#define BOLD { QFont f(painter->font());\
    f.setBold(true); \
    painter->setFont(f); }

#endif // MACROS_H
