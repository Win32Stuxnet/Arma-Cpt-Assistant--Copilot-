# AI Copilot Plugin for Arma Enfusion Workbench

This repository delivers a Workbench plugin that exposes a conversational AI copilot inside Arma Reforger's Enfusion tools. The plugin communicates with a local bridge service to relay prompts to Anthropic, OpenAI, or a self-hosted Ollama model and returns structured responses that can be inserted directly into the Script Editor.

## Features

- **Workbench integration** – launch the copilot from the Workbench plugin browser (default shortcut: `Ctrl` + `Shift` + `A`).
- **Chat-style requests** – describe problems or feature ideas in natural language; the plugin captures optional script selections to build context-rich prompts.
- **Multiple request modes** – general chat, code generation, analysis, debugging, documentation, optimisation, explanation, and refactoring.
- **File-bridge transport** – avoids direct HTTP calls from Enforce Script by exchanging JSON files with an external Node.js service.
- **Configurable services** – switch between Anthropic Claude, OpenAI ChatGPT, or local Ollama models. A custom endpoint mode is also available for bespoke deployments.
- **Persistent settings** – API keys, model names, bridge file paths, history preferences, and temperature/max-token limits are stored in `$profile:AIAssistantConfig.json`.

## Repository Layout

- `Scripts/WorkbenchGame/AIAssistant` – Enforce Script sources for the Workbench plugin (core logic, UI, settings, and service callbacks).
- `bridge-service/` – Node.js bridge responsible for calling AI providers, enforcing rate limits, and reading/writing the request/response files consumed by the plugin.
- `dashboard/` – Ancillary dashboard assets (unchanged by this update).

## Getting Started

### 1. Install the Workbench plugin

1. Copy the `Scripts/WorkbenchGame/AIAssistant` directory into your Enfusion project (or reference this repository directly in your mod workspace).
2. Launch the Enfusion Workbench and open the **Plugins** panel.
3. Locate **AI Assistant** under the *AI Tools* category and enable it. The plugin can be invoked with `Ctrl` + `Shift` + `A` or through the plugin list.

### 2. Run the bridge service

The bridge converts file-based requests into HTTP calls that the AI providers understand.

```bash
cd bridge-service
npm install
npm start
```

By default the bridge watches `../data/ai_request.json` and `../data/ai_response.json`. To match the Workbench profile directory you can either set the `ARMA_PROFILE_PATH` environment variable **or** update the plugin settings to point to the desired request/response files (see below).

### 3. Configure the plugin

1. Launch the plugin via `Ctrl` + `Shift` + `A` and open **Settings**.
2. Choose a service provider:
   - **Claude API** (Anthropic)
   - **OpenAI API**
   - **Local model** (Ollama compatible)
   - **Custom endpoint** (treated as OpenAI-style payloads – provide the endpoint URL and API key)
3. Enter the required API key or custom endpoint URL.
4. Adjust optional parameters:
   - Model name
   - Temperature (0.0 – 2.0)
   - Max tokens
   - Request/response file locations (defaults use `$profile:` so they follow your Workbench profile path)
   - History retention and UI preferences
5. Settings persist inside `$profile:AIAssistantConfig.json`.

### 4. Send a request

1. Select text in the Script Editor (optional) to provide context.
2. Invoke the plugin (`Ctrl` + `Shift` + `A`).
3. Choose a request type (e.g., *Generate code*, *Debug code*, *General chat*) and enter your prompt.
4. Submit the request. The plugin writes the JSON payload to the configured request file and polls for the bridge response.
5. When the bridge returns a result it is shown in Workbench. Generated code can optionally be inserted directly into the editor.

## Bridge Payload Format

The plugin writes a JSON document similar to the following:

```json
{
  "service": "openai",
  "prompt": "Explain the selected component",
  "model": "gpt-4o-mini",
  "settings": {
    "maxTokens": 2000,
    "temperature": 0.2,
    "timeout": 60000,
    "apiKey": "sk-...",
    "request_file": "$profile:ai_request.json",
    "response_file": "$profile:ai_response.json"
  },
  "metadata": {
    "requestType": "AIRequestType.CODE_ANALYSIS",
    "context": "Module=ScriptEditor, Script=Scripts/MyComponent.c"
  }
}
```

The bridge reads this file, performs the HTTP call, and writes a matching response document containing either the AI output or error details.

## Troubleshooting

- **No response / timeout** – ensure the Node.js bridge is running and that the plugin and bridge share the same request/response paths. The settings dialog exposes these paths for quick verification.
- **API key errors** – confirm that the key is valid and that the correct provider is selected. The bridge now accepts keys supplied directly by the plugin; environment variables remain a fallback.
- **Custom endpoint support** – custom endpoints are treated as OpenAI-compatible chat completions. Supply the full URL and API key in the plugin settings.
- **History disabled** – if you turn off history persistence the plugin only keeps the latest request in memory.

## License

This project is licensed under the terms specified in `LICENSE`.
