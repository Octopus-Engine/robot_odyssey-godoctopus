
	// declare_trigger_system<HealAndConsumeRuneLoadOnHit, trigger_module::DamageReceived, RuneCondition<1, DefaultRune>, HitpointEvent<5,1>>(ecs);
	// declare_trigger_system<HealAndConsumeRuneLoadOnAttack, trigger_module::Attack, RuneCondition<1, DefaultRune>, HitpointEvent<5,1>>(ecs);
	// declare_trigger_system<HealAndConsumeRuneLoadOnAttackTier2, trigger_module::Attack, RuneCondition<2, DefaultRune>, HitpointEvent<10,2>>(ecs);

	// declare_attack_trigger_system<BonusDamageConsumeRuneOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, FactorDamageEvent<20, 2>>(ecs);
	// declare_attack_trigger_system<BonusDamageRune, trigger_module::DamageDealt, AlwaysCondition, FactorDamageEventSpecialScaled<10, 1>>(ecs);
	// declare_attack_trigger_system<HasLowHpDoubleDamageRune, trigger_module::DamageDealt, HitPointUnderPercentCondition<25>, FactorDamageEvent<100, 25>>(ecs);
	// declare_attack_trigger_system<HasHighHpBonusDamageRune, trigger_module::DamageDealt, HitPointOverPercentCondition<75>, FactorDamageEvent<50, 25>>(ecs);

	// declare_attack_trigger_system<BonusDamageSelfDamage, trigger_module::DamageDealt, HitPointCostCondition<2>, FactorDamageEvent<150, 50>>(ecs);

	// declare_area_trigger_system<AoeDamageOnHit, trigger_module::DamageReceived, AlwaysCondition, DamageAreaEvent<1, 5, 1>>(ecs, ctx);
	// declare_area_trigger_system<AoeDamageOnHitLevel, trigger_module::DamageReceived, AlwaysCondition, DamageAreaEvent<1, 5, 1>>(ecs, ctx);

	// declare_area_trigger_system<AoeDamageConsumeRuneOnHitTier2, trigger_module::DamageReceived, RuneCondition<2, DefaultRune>, DamageAreaEvent<4, 5, 2>>(ecs, ctx);

	// declare_area_trigger_system<AoeHealOnHit, trigger_module::DamageReceived, AlwaysCondition, HealAreaEvent<2, 5, 1>>(ecs, ctx);

	// declare_area_trigger_system<AoeHealConsumeRuneOnHit, trigger_module::DamageReceived, RuneCondition<1, DefaultRune>, HealAreaEventSpecialScaled<4, 5, 2>>(ecs, ctx);
	// declare_area_trigger_system<AoeHealConsumeRuneOnHitTier2, trigger_module::DamageReceived, RuneCondition<2, DefaultRune>, HealAreaEvent<8, 5, 4>>(ecs, ctx);

	// declare_area_trigger_system<AoeHealOnAttack, trigger_module::DamageDealt, AlwaysCondition, HealAreaEvent<8, 2, 4>>(ecs, ctx);

	// declare_attack_area_trigger_system<AoeDamageSpecial, trigger_module::DamageDealt, AlwaysCondition, DamageAreaEventSpecialScaled<10, 5, 1>>(ecs, ctx);
	// declare_attack_area_trigger_system<AoeDamageConsumeRuneOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, DamageAreaEvent<2, 5, 1>>(ecs, ctx);
	// declare_attack_area_trigger_system<AoeDamageConsumeRuneOnAttackTier2, trigger_module::DamageDealt, RuneCondition<2, DefaultRune>, DamageAreaEvent<4, 5, 2>>(ecs, ctx);

	// declare_area_trigger_system<AoeRuneToEnnemiesOnHit, trigger_module::DamageReceived, AlwaysCondition, RuneAreaEvent<DefaultRune, 1, 5, false>>(ecs, ctx);

	// declare_area_trigger_system<AoeRuneToAlliesOnHit, trigger_module::DamageReceived, AlwaysCondition, RuneAreaEvent<DefaultRune, 1, 5, true>>(ecs, ctx);

	// declare_attack_trigger_target_condition_system<TargetHasHighHpBonusDamageRune, trigger_module::DamageDealt, HitPointOverPercentCondition<75>, FactorDamageEvent<50, 10>>(ecs);
	// declare_attack_trigger_target_condition_system<TargetHasHighHpBonusDamageRuneTier2, trigger_module::DamageDealt, HitPointOverPercentCondition<75>, FactorDamageEvent<100, 20>>(ecs);
	// declare_attack_trigger_target_condition_system<BonusDamageConsumeRuneOnTargetOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, FactorDamageEvent<30, 3>>(ecs);
	// declare_attack_trigger_target_condition_system<BonusDamageConsumeRuneOnTargetOnAttackTier2, trigger_module::DamageDealt, RuneCondition<2, DefaultRune>, FactorDamageEvent<200, 50>>(ecs);
	// declare_attack_trigger_target_condition_system<BonusDamageConsumeRuneOnTargetOnAttackTier3, trigger_module::DamageDealt, RuneCondition<3, DefaultRune>, FactorDamageEvent<300, 100>>(ecs);

	// declare_attack_area_trigger_target_condition_system<AoeDamageConsumeRuneOnTargetOnAttack, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, DamageAreaEvent<2, 5, 1>>(ecs, ctx);
	// declare_attack_area_trigger_target_condition_system<AoeDamageConsumeRuneOnTargetOnAttackTier2, trigger_module::DamageDealt, RuneCondition<2, DefaultRune>, DamageAreaEvent<4, 5, 2>>(ecs, ctx);

	// declare_attack_trigger_system<LifestealRuneCore, trigger_module::DamageDealt, AlwaysCondition, LifestealEventSpecialScaled<5, 1> >(ecs);	// declare_area_trigger_system<AoeDamageConsumeRuneOnDeath, trigger_module::Death, AlwaysCondition, DamageAreaEvent<8, 5, 8>>(ecs, ctx);
	// declare_area_trigger_system<AoeHealConsumeRuneOnDeath, trigger_module::Death, RuneCondition<1, DefaultRune>, HealAreaEvent<15, 5, 10>>(ecs, ctx);
	// declare_area_trigger_system<AddAoeRuneLoadToEnemiesOnDeath, trigger_module::Death, RuneCondition<1, DefaultRune>, RuneAreaEvent<DefaultRune, 1, 5, false>>(ecs, ctx);
	// declare_area_trigger_system<AddAoeRuneLoadToAlliesOnDeath, trigger_module::Death, AlwaysCondition, RuneAreaEvent<DefaultRune, 1, 5, true>>(ecs, ctx);
	// declare_attack_trigger_target_condition_system<LifestealRuneConsumeRuneOnTarget, trigger_module::DamageDealt, RuneCondition<1, DefaultRune>, LifestealEvent<20, 4> >(ecs);
