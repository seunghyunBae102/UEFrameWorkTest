// GHOST IN THE MONITOR - Inner World Equipment Data Asset
#include "Data/DA_InnerEquipment.h"

float UDA_InnerEquipment::ScalingToMultiplier(EStatScaling Scaling)
{
	switch (Scaling)
	{
	case EStatScaling::S:
		return 1.8f;  // S등급: 180%
	case EStatScaling::A:
		return 1.5f;  // A등급: 150%
	case EStatScaling::B:
		return 1.2f;  // B등급: 120%
	case EStatScaling::C:
		return 1.0f;  // C등급: 100%
	case EStatScaling::D:
		return 0.8f;  // D등급: 80%
	case EStatScaling::None:
	default:
		return 0.0f;  // 보정 없음
	}
}
