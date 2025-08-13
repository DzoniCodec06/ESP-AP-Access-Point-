const socket = new WebSocket(`ws://${window.location.hostname}:81/`);

socket.addEventListener("open", (event) => {
    socket.send("wev server opend");
    console.log("Server opened");
});

socket.addEventListener("message", (event) => {
    console.log("message from server", event.data);
})

const solidBtn = document.getElementById("solidBtn");
const animBtn = document.getElementById("animBtn");

const container = document.getElementById("cont");
const solidSelection = document.getElementById("solidSelection");

const arrayBtn0 = document.getElementById("array-0");
const arrayBtn1 = document.getElementById("array-1");
const arrayBtn2 = document.getElementById("array-2");

let array0 = false;
let array1 = false;
let array2 = false;

let animSelected = false;

solidBtn.addEventListener("click", () => {
    container.classList.replace("container", "container-false");
    solidSelection.classList.replace("solid-false", "solid-true");
    solidBtn.classList.add("s");
    animBtn.classList.remove("s");
})

animBtn.addEventListener("click", () => {
    container.classList.replace("container-false", "container");
    solidSelection.classList.replace("solid-true", "solid-false");
    solidBtn.classList.remove("s");
    animBtn.classList.add("s");
})


arrayBtn0.addEventListener("click", () => {
    array0 = !array0;

    sendMessage("array-0");

    array0 == true ? arrayBtn0.classList.replace("animation-btn", "animation-btn-s") : arrayBtn0.classList.replace("animation-btn-s", "animation-btn"); 
    arrayBtn1.classList.replace("animation-btn-s", "animation-btn");
    arrayBtn2.classList.replace("animation-btn-s", "animation-btn");

    array1 = false;
    array2 = false;
})

arrayBtn1.addEventListener("click", () => {
    array1 = !array1;

    sendMessage("array-1");

    arrayBtn0.classList.replace("animation-btn-s", "animation-btn");
    array1 == true ? arrayBtn1.classList.replace("animation-btn", "animation-btn-s") : arrayBtn1.classList.replace("animation-btn-s", "animation-btn");
    arrayBtn2.classList.replace("animation-btn-s", "animation-btn");

    array0 = false;
    array2 = false;
})

arrayBtn2.addEventListener("click", () => {
    array2 = !array2;

    sendMessage("array-2");

    arrayBtn0.classList.replace("animation-btn-s", "animation-btn");
    arrayBtn1.classList.replace("animation-btn-s", "animation-btn");
    array2 == true ? arrayBtn2.classList.replace("animation-btn", "animation-btn-s") : arrayBtn2.classList.replace("animation-btn-s", "animation-btn");
    
    array0 = false;
    array1 = false;
})

socket.onopen = () => {
  console.log("Connected to ESP32");
};

socket.onmessage = (event) => {
  console.log("Received: " + event.data);
};

socket.onclose = () => {
  console.log("Disconnected");
};

function sendMessage(msg) {
  if (socket.readyState === WebSocket.OPEN) {
    socket.send(msg);
    console.log(`Sent: ${msg}`);
  }
}
