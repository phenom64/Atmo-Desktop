#ifndef MACROS_H
#define MACROS_H

#define CCSize CC_MdiControls+1
#define CESize CE_ShapedFrame+1
#define PESize PE_PanelMenu+1
#define EVSize QEvent::PlatformPanel+1

#define SUNKEN state & (State_Sunken | State_Selected | State_On)
#define HOVER state & State_MouseOver
#define ENABLED state & State_Enabled

#define castOpt(_Type_, _varName_, _fromVar_) const QStyleOption##_Type_ *_varName_ = qstyleoption_cast<const QStyleOption##_Type_ *>(_fromVar_)
#define castObj(_Type_, _varName_, _fromVar_) _Type_ _varName_ = qobject_cast<_Type_>(_fromVar_)

#define sAdjusted(_X1_, _Y1_, _X2_, _Y2_) adjusted(bool(sides&Render::Left)*_X1_, bool(sides&Render::Top)*_Y1_, bool(sides&Render::Right)*_X2_, bool(sides&Render::Bottom)*_Y2_)
#define sAdjust(_X1_, _Y1_, _X2_, _Y2_) adjust(bool(sides&Render::Left)*_X1_, bool(sides&Render::Top)*_Y1_, bool(sides&Render::Right)*_X2_, bool(sides&Render::Bottom)*_Y2_)
#define sShrink(_S_) sAdjust(_S_, _S_, -_S_, -_S_)
#define sShrinked(_S_) sAdjusted(_S_, _S_, -_S_, -_S_)
#define shrinked(_S_) adjusted(_S_, _S_, -_S_, -_S_)
#define shrink(_S_) adjust(_S_, _S_, -_S_, -_S_)

#define ISA(_TYPE_) inherits("##_TYPE_##")

#define BOLD QFont f(painter->font());\
    f.setBold(true); \
    painter->setFont(f)

#endif // MACROS_H
