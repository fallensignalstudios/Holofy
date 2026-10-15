#pragma once

#include "CoreMinimal.h"

/** Builds the M_Holofy_* master materials and MI_Holofy_* preset instances inside the plugin's content folder. */
namespace HolofyMaterialBuilder
{
	/** (Re)generates every asset and saves them. Returns true on success. */
	bool GenerateAll();

	/** True if the master materials already exist on disk. */
	bool AreMaterialsGenerated();
}
