#include "Testing/EntityTestingModule.h"
#include "Testing/TestCharacters.h"
#include "Core/GameWorld.h"

#include <EASTL/array.h>
#include <EASTL/stack.h>

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogTestingEntity);

	namespace
	{
		template <typename T>
		bool CompareValuesEq(const T& inValue)
		{
			UNREFERENCED_PARAMETER(inValue);
			return true;
		}

		template <typename T>
		bool CompareValuesEq(const T& inFirstValue, const T& inSecondValue)
		{
			return inFirstValue == inSecondValue;
		}

		template <typename T, typename ...Args>
		bool CompareValuesEq(const T& inFirstValue, const T& inNextValue, const Args&... inRestValues)
		{
			return (inFirstValue == inNextValue) && CompareValuesEq(inFirstValue, inRestValues...);
		}
	}

	namespace Test
	{
		bool TestEntityModule(OGameWorld& inTestingGameWorld)
		{
			bool result = true;

			result = TestEntityRootAttachment(inTestingGameWorld);

			return result;
		}

		bool TestEntityRootAttachment(OGameWorld& inTestingGameWorld)
		{
			enum ERootAssignmentOrder
			{
				ERootAssignment_Pre,
				ERootAssignment_Mid,
				ERootAssignment_Post,
				ERootAssignment_None,
				ERootAssignment_MAX
			};

			eastl::array<eastl::queue<ULinearTransform>, ERootAssignment_MAX> entityTransformHierarchyQueue;
			eastl::array<ULinearTransform, ERootAssignment_MAX> currentTransformsContainer;

			const size_t numTransformStacks = entityTransformHierarchyQueue.size();

			auto preRootAssignedChar = inTestingGameWorld.SpawnEntity<APreRootAssignedCharacter>();
			auto midRootAssignedChar = inTestingGameWorld.SpawnEntity<AMidRootAssignedCharacter>();
			auto postRootAssignedChar = inTestingGameWorld.SpawnEntity<APostRootAssignedCharacter>();
			auto noRootAssignedChar = inTestingGameWorld.SpawnEntity<ANoRootAssignedCharacter>();

			// Perform checks on the transform hierarchy of all of the characters and make sure that they're the same
			preRootAssignedChar->PopulateTransformQueue(entityTransformHierarchyQueue[ERootAssignment_Pre]);
			midRootAssignedChar->PopulateTransformQueue(entityTransformHierarchyQueue[ERootAssignment_Mid]);
			postRootAssignedChar->PopulateTransformQueue(entityTransformHierarchyQueue[ERootAssignment_Post]);
			noRootAssignedChar->PopulateTransformQueue(entityTransformHierarchyQueue[ERootAssignment_None]);

			size_t preStackSize = entityTransformHierarchyQueue[ERootAssignment_Pre].size();
			size_t midStackSize = entityTransformHierarchyQueue[ERootAssignment_Mid].size();
			size_t postStackSize = entityTransformHierarchyQueue[ERootAssignment_Post].size();
			size_t noneStackSize = entityTransformHierarchyQueue[ERootAssignment_None].size();

			if (!CompareValuesEq(preStackSize, midStackSize, postStackSize, noneStackSize))
			{
				return false;
			}

			const size_t numTransforms = entityTransformHierarchyQueue[ERootAssignment_Pre].size();

			for (size_t i = 0; i < numTransforms; ++i)
			{
				// Pop off the top linear transform from each hierarchy stack
				for (int j = 0; j < ERootAssignment_MAX; ++j)
				{
					currentTransformsContainer[j] = entityTransformHierarchyQueue[j].front();
					entityTransformHierarchyQueue[j].pop();
				}

				// Popped off the current top layer of all stacks into currentTransformsContainer, we need to check if they're all equal to each other
				const ULinearTransform currentTargetTransform = currentTransformsContainer.front();

				for (size_t k = 0; k < currentTransformsContainer.size(); ++k)
				{
					//LOG(LogTestingEntity, Log, "Transform #%d for Stack %d: %s\n", i + 1, k + 1, currentTransformsContainer[k].ToString().c_str());

					if (currentTransformsContainer[k] != currentTargetTransform)
					{
						// The transform is not the same, meaning the hierarchy is different!!!
						return false;
					}
				}
			}

			return true;
		}
	}
}