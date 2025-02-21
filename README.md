# QNX Timer with CURL - Fetch Quote Example

This C program demonstrates how to use the QNX timer system in conjunction with libcurl to fetch a quote from an external API (Kanye REST API) periodically.

## Main points
- Creates a timer using `timer_create()` and pulses to handle periodic events.
- Uses libcurl to fetch a quote from the Kanye REST API.
- Extracts and prints the quote from a JSON response.

## Dependencies
- **QNX Neutrino**: The program is designed to work in a QNX environment.
- **libcurl**: For making HTTP requests to the Kanye API.
