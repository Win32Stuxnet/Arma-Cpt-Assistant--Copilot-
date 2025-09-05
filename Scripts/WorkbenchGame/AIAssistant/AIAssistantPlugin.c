//-----------------------------------------------------------------------------
//! AI Assistant Plugin for Arma Reforger Workbench
//! Provides AI-powered development assistance directly in the editor
//-----------------------------------------------------------------------------

[WorkbenchPluginAttribute(
	name: "AI Assistant", 
	description: "AI-powered development assistant with code generation, analysis, and debugging help",
	wbModules: {"ResourceManager", "ScriptEditor", "WorldEditor"}, 
	category: "AI Tools",
	shortcut: "Ctrl+Shift+A",
	awesomeFontCode: 0xF085
)]
class AIAssistantPlugin : WorkbenchPlugin
{
	protected ref AIAssistantCore m_AICore;
	protected ref AIAssistantUI m_UI;
	protected ref AIAssistantSettings m_Settings;
	
	//-----------------------------------------------------------------------------
	//! Plugin initialization
	override void Configure()
	{
		m_Settings = new AIAssistantSettings();
		m_AICore = new AIAssistantCore(m_Settings);
		m_UI = new AIAssistantUI(m_AICore);
	}
	
	//-----------------------------------------------------------------------------
	//! Main plugin entry point
	override void Run()
	{
		// Check if plugin is configured
		if (!m_Settings.IsConfigured())
		{
			ShowConfigurationDialog();
			return;
		}
		
		// Show main AI Assistant interface
		ShowAIAssistantDialog();
	}
	
	//-----------------------------------------------------------------------------
	//! Show configuration dialog for first-time setup
	void ShowConfigurationDialog()
	{
		string configText = "AI Assistant Plugin Configuration\n\n";
		configText += "This plugin provides AI-powered assistance for Arma Reforger development.\n\n";
		configText += "Features:\n";
		configText += "• Code generation from natural language\n";
		configText += "• Script analysis and debugging help\n";
		configText += "• Documentation generation\n";
		configText += "• Performance optimization suggestions\n\n";
		configText += "Would you like to configure the plugin now?";
		
		if (Workbench.Dialog("AI Assistant Setup", configText, MessageBoxButtons.YESNO) == IDYES)
		{
			OpenSettingsDialog();
		}
	}
	
	//-----------------------------------------------------------------------------
	//! Show main AI Assistant dialog
	void ShowAIAssistantDialog()
	{
		if (m_UI)
			m_UI.ShowMainDialog();
	}
	
	//-----------------------------------------------------------------------------
	//! Open plugin settings
	void OpenSettingsDialog()
	{
		if (m_UI)
			m_UI.ShowSettingsDialog();
	}
	
	//-----------------------------------------------------------------------------
	//! Get current workbench context
	WorkbenchContext GetCurrentContext()
	{
		WorkbenchContext context = new WorkbenchContext();
		
		// Get current module
		context.currentModule = Workbench.GetModule();
		
		// Get selected resources if in Resource Manager
		if (context.currentModule == "ResourceManager")
		{
			array<ResourceName> selectedResources = {};
			Workbench.GetResourceManager().GetSelection(selectedResources);
			context.selectedResources = selectedResources;
		}
		
		// Get current script if in Script Editor
		if (context.currentModule == "ScriptEditor")
		{
			context.currentScript = Workbench.ScriptDialog().GetCurrentScript();
		}
		
		// Get selected entities if in World Editor
		if (context.currentModule == "WorldEditor")
		{
			context.selectedEntities = Workbench.GetWorldEditor().GetSelection();
		}
		
		return context;
	}
}

//-----------------------------------------------------------------------------
//! Context information for AI operations
class WorkbenchContext
{
	string currentModule;
	array<ResourceName> selectedResources;
	string currentScript;
	array<ref IEntity> selectedEntities;
	vector worldPosition;
	
	void WorkbenchContext()
	{
		selectedResources = {};
		selectedEntities = {};
		worldPosition = vector.Zero;
	}
}