#ifndef HK_PHYSICS_LOCAL_CONSTRAINT_SYSTEM_H
#define HK_PHYSICS_LOCAL_CONSTRAINT_SYSTEM_H

class hk_Local_Constraint_System_BP;

// IVP_EXPORT_PUBLIC

class IVP_Core;

class hk_Local_Constraint_System : public hk_Link_EF
{
public:
	hk_Local_Constraint_System( hk_Environment *, hk_Local_Constraint_System_BP* );
	//: creates a deactivated empty constraint system


	void activate();
	void deactivate();
	void deactivate_silently();

	//: activates the constraint

	virtual ~hk_Local_Constraint_System();

	inline hk_Environment* get_environment() const
	{
		return m_environment;
	};

public:	// internal
	virtual void entity_deletion_event(hk_Entity *);

	void core_is_going_to_be_deleted_event(IVP_Core *);

	void constraint_deletion_event( hk_Constraint * );

	virtual hk_effector_priority get_effector_priority(){
		return HK_PRIORITY_LOCAL_CONSTRAINT_SYSTEM;
	}

	void get_effected_entities(hk_Array<hk_Entity*> &ent_out);
		
	//virtual hk_real get_minimum_simulation_frequency(hk_Array<hk_Entity> *);

	void apply_effector_PSI(	hk_PSI_Info&, hk_Array<hk_Entity*>* );

	void apply_effector_collision(	hk_PSI_Info&,	hk_Array<hk_Entity*>* ){ ;}

	hk_real get_epsilon();
	inline bool is_active() const { return m_is_active; }

	inline void set_client_data( void *client_data ) { m_client_data = client_data; }
	inline void *get_client_data() const { return m_client_data; }

	//: get the epsilon which defines the softness of the constraint
protected:
	friend class hk_Constraint;

	void add_constraint( hk_Constraint *, int storage_size);

	//: adds a constraint to the constraint system,
	//: Note: if the constraint is deactivated, than adding constraints is much faster

	void recalc_storage_size();
	int							m_n_iterations;
	int							m_size_of_all_vmq_storages;
	hk_Array<hk_Constraint *> 	m_constraints;
	hk_Array<hk_Rigid_Body*>	m_bodies;
	bool						m_is_active;

private:
	void *m_client_data;
};

//#include <hk_physics/constraint/local_constraint_system/local_constraint_system.inl>

#endif /* HK_PHYSICS_LOCAL_CONSTRAINT_SYSTEM_H */

