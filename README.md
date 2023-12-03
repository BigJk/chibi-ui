<p align="center">
    <img src="./github/logo.png" width="64" height="64" />
</p>

# 

<p align="center">
    <img src="./github/screenshot.png" />
</p>

``chibi-ui`` is a highly experimental and opinionated UI *"framework"* to create native desktop applications using web technologies, while avoiding shipping a browser with the application or using any kind of webview. It uses [quickjs](https://github.com/quickjs-ng/quickjs) as the JavaScript engine and makes [ImGui](https://github.com/ocornut/imgui) available to the JavaScript code.

# Motivation

Dealing with frontend since the time when JQuery was the way to go I grew very fond of the ui ecosystem of the web. I like the style and layout abilities we have in modern browsers, but I dislike the fact that we need to ship the browser with the application (e.g. electron) or rely on a webview being available on the target platform (e.g. tauri) if we want to use web technologies to create desktop applications. Even without shipping the browser the memory usage of these applications is still very high even for basic applications.

This project is a highly opinionated experiment to see which concepts of the web can be distilled into a performant native ui framework and if this can be done without sacrificing the things I like about the web.

### What I want to keep

- A small set of great layout concepts (e.g. Flexbox, Grid)
- Familiar styling (e.g. CSS-like)
- JavaScript as the main language, because of the big ecosystem of libraries and frameworks
- Easy cross-platform support

### What I want to avoid

- Shipping a browser with the application or using a webview
- High memory usage
- Different ways to do the same thing (e.g. [some historical layouting methods of the web](https://i.imgur.com/1ERqMX7.png))

# Roadmap

## Phase 1: Low-Level ImGui Bindings

In the first phase of the project, the goal is to create low-level bindings to ImGui, so that it can be used from JavaScript. This will allow the creation of simple UI application that can be used to test the bindings and the JavaScript engine. The bindings are very "stiff" and do not provide any kind of abstraction over the ImGui API.

## Phase 2: High-Level JavaScript API

In the second phase of the project, the goal is to create a high-level JavaScript API build on top of the low-level bindings. This API will provide a more JavaScript-like interface to the ImGui API and syntax sugar to make it easier to use.

The goal will be to define custom components and more complex layouts with the least amount of code possible, while still being able to use the full power of ImGui.

## Phase 3: Optimizations

In the third phase of the project, the goal is to optimize the rendering of the UI. One possible approach might be to only re-render the UI when relevant state changes.

# Examples

The following example creates a simple window with a button that prints a message to console when pressed. The ``globalThis.tick`` function is called every frame.

```javascript
globalThis.tick = () => {
    imgui.begin("Hello World", 0);
    {
        if (imgui.button('Test Button')) { print("button pressed!") }
    }
    imgui.end();
    
    imgui.showMetricsWindow();
}
```

See ``examples/`` for more examples

# CLI

```
chibi-ui <entrypoint.js>
```

# Building

...

# Icon Credits

<a href="https://www.flaticon.com/free-icons/cute-food" title="cute food icons">Cute food icons created by Freepik - Flaticon</a>