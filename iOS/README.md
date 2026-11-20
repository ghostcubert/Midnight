# Midnight - iOS

**Midnight - iOS** is a **NSURLProtocol** redirect that registers itself to the game and redirect every request from epicgames to a Hosted Custom Backend to give the ability to play on iOS.

## Support

- **3.3 - 10.30**: Fully working
- **31.00+**: Experimental/Not tested

## How to use

Fork this repository and change the **BACKEND_URL** in ``includes/opts.h`` and build from source if you are on a mac or commit and you will find the .dylib in a zip. Use it with an injecting tool to inject the dylib into your IPA.
