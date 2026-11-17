import unreal
for p in ['/Game/_Workspace/6_Room/DA_StageData','/Game/_Workspace/6_Room/DA_RoomMonsterSpawn']:
    a=unreal.load_asset(p)
    unreal.log('ASSET '+p+' = '+str(a))
    if not a: continue
    if 'DA_StageData' in p:
        for i,s in enumerate(a.get_editor_property('stages')):
            unreal.log('STAGE '+str(i+1)+' '+str(s.get_editor_property('stage_id'))+' rooms='+str(s.get_editor_property('room_count'))+' seed_random='+str(s.get_editor_property('b_use_random_seed'))+' seed='+str(s.get_editor_property('fixed_seed')))
    else:
        for i,r in enumerate(a.get_editor_property('spawn_rules')):
            unreal.log('RULE '+str(i+1)+' type='+str(r.get_editor_property('room_type'))+' score='+str(r.get_editor_property('score_range')))
            for e in r.get_editor_property('monster_pool'):
                cls=e.get_editor_property('monster_class')
                unreal.log('MONSTER '+str(cls.get_name() if cls else None)+' cost='+str(e.get_editor_property('cost'))+' weight='+str(e.get_editor_property('weight'))+' max='+str(e.get_editor_property('max_count')))
