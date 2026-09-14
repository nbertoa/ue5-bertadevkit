#pragma once

#include "CoreTypes.h"

namespace BertaGSCAbilityQueueInputBridgePrivate
{
	enum class EInputFailureDecision : uint8
	{
		Accept,
		BridgeInactive,
		InvalidRequest,
		QueueUnavailable,
		BindingMismatch,
		AbilityNotAllowed
	};

	struct FInputFailureContext
	{
		bool bBridgeActive = false;
		bool bRequestValid = false;
		bool bQueueEnabledAndOpen = false;
		bool bRuntimeBindingMatches = false;
		bool bAbilityAllowed = false;
	};

	inline EInputFailureDecision EvaluateInputFailure(const FInputFailureContext& Context)
	{
		if (!Context.bBridgeActive)
		{
			return EInputFailureDecision::BridgeInactive;
		}
		if (!Context.bRequestValid)
		{
			return EInputFailureDecision::InvalidRequest;
		}
		if (!Context.bQueueEnabledAndOpen)
		{
			return EInputFailureDecision::QueueUnavailable;
		}
		if (!Context.bRuntimeBindingMatches)
		{
			return EInputFailureDecision::BindingMismatch;
		}
		if (!Context.bAbilityAllowed)
		{
			return EInputFailureDecision::AbilityNotAllowed;
		}
		return EInputFailureDecision::Accept;
	}
}
