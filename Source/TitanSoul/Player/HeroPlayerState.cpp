// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroPlayerState.h"
#include "../Attributes/HeroAttributeSet.h"
#include "../AbilitySystem/HeroAbilitySystemComponent.h"

AHeroPlayerState::AHeroPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UHeroAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another GDPlayerState (Hero) receives a GE,
	// we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create the attribute set, this replicates by default
	// Adding it as a subobject of the owning actor of an AbilitySystemComponent
	// automatically registers the AttributeSet with the AbilitySystemComponent
	AttributeSetBase = CreateDefaultSubobject<UHeroAttributeSet>(TEXT("AttributeSetBase"));
}

// Implement IAbilitySystemInterface
UAbilitySystemComponent* AHeroPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UHeroAttributeSet* AHeroPlayerState::GetAttributeSetBase() const
{
	return AttributeSetBase;
}

void AHeroPlayerState::InitializeAttributes()
{

}
