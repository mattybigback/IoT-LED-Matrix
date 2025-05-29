document.addEventListener('DOMContentLoaded', () => {
    // Fetch initial data from the API
    fetch('/api/message')
        .then(r => r.json())
        .then(s => {
            // Populate the form with server values
            document.getElementById('msg').value = s.message;
            document.getElementById('msg').maxlength = s.message_max_len;
            document.getElementById('msg').setAttribute('maxlength', s.message_max_len);
            document.getElementById('intensity').value = s.intensity;
            document.getElementById('intensityValue').innerHTML = s.intensity; // Display initial intensity value
            document.getElementById('intensity').min = s.intensity_min;
            document.getElementById('intensity').max = s.intensity_max;
            document.getElementById('speed').value = s.speed;
            document.getElementById('speed').min = s.speed_min;
            document.getElementById('speed').max = s.speed_max;
            document.getElementById('speedValue').innerHTML = s.speed; // Display initial speed value
            document.getElementById('displayFlipped').checked = s.display_flipped;
        })
        .catch(err => console.error('values.json fetch failed', err));

    // Add event listener to send data via PUT
    document.getElementById('messageParameters').addEventListener('submit', (e) => {
        e.preventDefault(); // Prevent form from reloading the page

        // Gather form data
        const data = {
            message: document.getElementById('msg').value,
            intensity: parseInt(document.getElementById('intensity').value, 10),
            speed: parseInt(document.getElementById('speed').value, 10),
            display_flipped: document.getElementById('displayFlipped').checked,
            change_immediately: document.getElementById('changeImmedately').checked
        };

        // Send data to the API using PUT
        fetch('/api/message', {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(data)
        })
            .then(response => {
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return response.json();
            })
            .then(result => {
                console.log('Data successfully updated:', result);
            })
            .catch(err => console.error('Failed to update data:', err));
    });
});

var intensitySlider = document.getElementById("intensity");
var intensityOutput = document.getElementById("intensityValue");
intensityOutput.innerHTML = intensitySlider.value; // Display the default slider value
intensitySlider.oninput = function() {
    intensityOutput.innerHTML = this.value; // Update the current slider value (each time you drag the slider handle)
}
var speedSlider = document.getElementById("speed");
var speedOutput = document.getElementById("speedValue");
speedOutput.innerHTML = speedSlider.value; // Display the default slider value
speedSlider.oninput = function() {
    speedOutput.innerHTML = this.value; // Update the current slider value (each time you drag the slider handle)
}