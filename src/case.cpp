#include <wx/datstrm.h>
#include "case.h"
#include "main.h"

Case::Case()
{

}

Case::Case( const wxString &tech, const wxString &fin )
{
	SetConfiguration( tech, fin );
}

Case::~Case()
{
	ClearListeners();
}
	
Object *Case::Duplicate()
{
	Case *c = new Case();
	c->Copy(this);
	return c;
}

bool Case::Copy( Object *obj )
{
	if ( Case *rhs = dynamic_cast<Case*>( obj ) )
	{
		m_technology = rhs->m_technology;
		m_financing = rhs->m_financing;
		m_vars.Copy( rhs->m_vars );
		m_baseCase.Copy( rhs->m_vars );
		m_properties = rhs->m_properties;
		m_notes = rhs->m_notes;
		return true;
	}
	else
		return false;
}

wxString Case::GetTypeName()
{
	return "sam.case";
}

void Case::Write( wxOutputStream &_o )
{
	wxDataOutputStream out(_o);

	out.Write8( 0x9b );
	out.Write8( 1 );

	// write data
	out.WriteString( m_technology );
	out.WriteString( m_financing );
	m_vars.Write( _o );
	m_baseCase.Write( _o );
	m_properties.Write( _o );
	m_notes.Write( _o );

	out.Write8( 0x9b );
}

bool Case::Read( wxInputStream &_i )
{
	wxDataInputStream in(_i);

	wxUint8 code = in.Read8();
	in.Read8(); // version

	// read data
	m_technology = in.ReadString();
	m_financing = in.ReadString();
	if ( !m_vars.Read( _i ) ) wxLogStatus("error reading m_vars in Case::Read");
	if ( !m_baseCase.Read( _i ) ) wxLogStatus("error reading m_baseCase in Case::Read");
	if ( !m_properties.Read( _i ) ) wxLogStatus("error reading m_properties in Case::Read");
	if ( !m_notes.Read( _i ) ) wxLogStatus("error reading m_notes in Case::Read");

	return (in.Read8() == code);
}


void Case::SetConfiguration( const wxString &tech, const wxString &fin )
{
	if ( m_technology == tech && m_financing == fin )
		return;

	m_technology = tech;
	m_financing = fin;

	// erase all input variables
	m_vars.clear();

	// erase results
	m_baseCase.clear();

	// look up configuration

	// set up all default variables and values

	// reevalute all equations
	
	// update UI
	SendEvent( CaseEvent( CaseEvent::CONFIG_CHANGED, tech, fin ) );
}

void Case::GetConfiguration( wxString *tech, wxString *fin )
{
	if ( tech ) *tech = m_technology;
	if ( fin ) *fin = m_financing;
}

int Case::Changed( const wxString &var )
{
	VarEvaluator eval( &m_vars, &SamApp::Vars() );
	int n = eval.Changed( var );
	if ( n > 0 ) SendEvent( CaseEvent( CaseEvent::VARS_CHANGED, eval.GetUpdated() ) );
	return n;

}

int Case::CalculateAll()
{
	VarEvaluator eval( &m_vars, &SamApp::Vars() );
	int n = eval.CalculateAll();
	if ( n > 0 ) SendEvent( CaseEvent( CaseEvent::VARS_CHANGED, eval.GetUpdated() ) );
	return n;
}

void Case::AddListener( CaseEventListener *cel )
{
	m_listeners.push_back( cel );
}

void Case::RemoveListener( CaseEventListener *cel )
{
	std::vector<CaseEventListener*>::iterator it = std::find( m_listeners.begin(), m_listeners.end() , cel );
	if ( it != m_listeners.end() )
		m_listeners.erase( it );
}

void Case::ClearListeners()
{
	m_listeners.clear();
}

void Case::SendEvent( CaseEvent e )
{
	for( size_t i=0;i<m_listeners.size();i++ )
		m_listeners[i]->OnCaseEvent( this, e );
}
