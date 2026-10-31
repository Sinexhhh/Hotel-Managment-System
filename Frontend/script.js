const TYPES = [
  { name: "Single Non-AC", price: 1000, prefix: 100 },
  { name: "Single AC", price: 1500, prefix: 200 },
  { name: "Double AC", price: 2500, prefix: 300 },
  { name: "Deluxe AC", price: 4000, prefix: 400 }
];

const FOODS = [
  { name: "Tea", price: 30, emoji: "🍵" },
  { name: "Coffee", price: 50, emoji: "☕" },
  { name: "Sandwich", price: 80, emoji: "🥪" },
  { name: "Burger", price: 100, emoji: "🍔" },
  { name: "Pizza", price: 180, emoji: "🍕" },
  { name: "Pasta", price: 150, emoji: "🍝" },
  { name: "Rice", price: 100, emoji: "🍚" },
  { name: "Dal", price: 120, emoji: "🥣" },
  { name: "Paneer", price: 180, emoji: "🧀" },
  { name: "Cold Drink", price: 40, emoji: "🥤" }
];

let bookings = [];
let nextId = 1001;

try {
  const stored = JSON.parse(
    localStorage.getItem("grandStayBookings") || "[]"
  );

  if (Array.isArray(stored)) {
    bookings = stored;
    nextId = Math.max(
      1001,
      ...bookings.map(booking => Number(booking.id) + 1)
    );
  }
} catch (error) {
  bookings = [];
}

const money = value =>
  "₹" + Number(value || 0).toLocaleString("en-IN");

const esc = value =>
  String(value).replace(/[&<>"']/g, character => ({
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': "&quot;",
    "'": "&#39;"
  })[character]);

const active = () => bookings.filter(booking => booking.active);

const roomLabel = booking => TYPES[booking.type].name;

const roomNo = (type, slot) => TYPES[type].prefix + slot;

function save() {
  localStorage.setItem(
    "grandStayBookings",
    JSON.stringify(bookings)
  );
}

function notify(message) {
  const notice = document.getElementById("notice");

  notice.textContent = message;
  notice.style.display = "block";

  clearTimeout(notify.timer);

  notify.timer = setTimeout(() => {
    notice.style.display = "none";
  }, 2600);
}

function available(type) {
  const occupied = new Set(
    active()
      .filter(booking => booking.type === type)
      .map(booking => booking.room)
  );

  return Array.from(
    { length: 5 },
    (_, index) => roomNo(type, index + 1)
  ).filter(room => !occupied.has(room));
}

function showPage(name) {
  document.querySelectorAll(".page").forEach(page => {
    page.classList.toggle("active", page.id === name);
  });

  document.querySelectorAll(".nav").forEach(button => {
    button.classList.toggle(
      "active",
      button.dataset.page === name
    );
  });

  const titles = {
    dashboard: "Dashboard Overview",
    rooms: "Room Directory",
    bookings: "Booking Management",
    food: "Food Orders",
    billing: "Checkout & Billing"
  };

  document.getElementById("title").textContent = titles[name];
}

document.querySelectorAll(".nav").forEach(button => {
  button.addEventListener("click", () => {
    showPage(button.dataset.page);
  });
});

function openBooking() {
  document.getElementById("booking-form").reset();
  document.getElementById("nights").value = 1;

  updateRoomNote();

  document.getElementById("booking-dialog").showModal();
}

function closeBooking() {
  document.getElementById("booking-dialog").close();
}

function updateRoomNote() {
  const type = Number(
    document.getElementById("room-type").value
  );

  const rooms = available(type);

  document.getElementById("room-note").textContent =
    rooms.length
      ? "Available rooms: " + rooms.join(", ")
      : "No rooms available in this category.";

  document.querySelector(
    "#booking-form button[type=submit]"
  ).disabled = !rooms.length;
}

document.getElementById("room-type").addEventListener(
  "change",
  updateRoomNote
);

document.getElementById("booking-form").addEventListener(
  "submit",
  event => {
    event.preventDefault();

    const name = document.getElementById("guest-name")
      .value.trim();

    const phone = document.getElementById("guest-phone")
      .value.trim();

    const type = Number(
      document.getElementById("room-type").value
    );

    const nights = Number(
      document.getElementById("nights").value
    );

    const rooms = available(type);

    if (!name || !/^[0-9+\-()\s]{7,20}$/.test(phone)) {
      notify("Enter a name and valid phone number.");
      return;
    }

    if (
      !Number.isInteger(nights) ||
      nights < 1 ||
      nights > 365
    ) {
      notify("Enter 1–365 nights.");
      return;
    }

    if (!rooms.length) {
      notify("No rooms available.");
      return;
    }

    bookings.push({
      id: nextId++,
      name,
      phone,
      type,
      room: rooms[0],
      nights,
      foodBill: 0,
      orders: [],
      active: true
    });

    save();
    closeBooking();
    render();

    notify("Booking created successfully!");
  }
);

function render() {
  document.getElementById("available-stat").textContent =
    TYPES.reduce(
      (total, _, index) => total + available(index).length,
      0
    );

  document.getElementById("booking-stat").textContent =
    active().length;

  document.getElementById("revenue-stat").textContent =
    money(
      active().reduce(
        (total, booking) =>
          total +
          TYPES[booking.type].price * booking.nights +
          booking.foodBill,
        0
      )
    );

  document.getElementById("summary").innerHTML =
    TYPES.map((type, index) => `
      <div class="summary">
        <strong>${available(index).length}/5</strong>
        ${type.name}
        <p>${available(index).length} available</p>
      </div>
    `).join("");

  document.getElementById("room-grid").innerHTML =
    TYPES.map((type, index) =>
      Array.from({ length: 5 }, (_, slot) => {
        const number = roomNo(index, slot + 1);

        const booking = active().find(
          item => item.room === number
        );

        return `
          <div class="room">
            <h3>Room ${number}</h3>
            <p>${type.name}</p>
            <strong>${money(type.price)}/night</strong>
            <p>
              ${
                booking
                  ? "Occupied: " + esc(booking.name)
                  : "Available"
              }
            </p>
          </div>
        `;
      }).join("")
    ).join("");

  renderBookings();
  renderRecent();
  renderFood();
  renderBillOptions();
}

function renderBookings() {
  const query = (
    document.getElementById("search").value || ""
  ).toLowerCase().trim();

  const rows = bookings
    .filter(booking =>
      [
        booking.id,
        booking.name,
        booking.phone,
        booking.room
      ].some(value =>
        String(value).toLowerCase().includes(query)
      )
    )
    .slice()
    .reverse();

  document.getElementById("booking-table").innerHTML =
    rows.length
      ? rows.map(booking => `
        <tr>
          <td>#${booking.id}</td>

          <td>
            ${esc(booking.name)}<br>
            <small>${esc(booking.phone)}</small>
          </td>

          <td>
            ${booking.room}<br>
            <small>${roomLabel(booking)}</small>
          </td>

          <td>${booking.nights}</td>
          <td>${money(booking.foodBill)}</td>

          <td>
            <span class="badge ${booking.active ? "" : "closed"}">
              ${booking.active ? "Active" : "Checked out"}
            </span>
          </td>

          <td>
            ${
              booking.active
                ? `<button
                    class="secondary"
                    onclick="checkout(${booking.id})"
                  >Checkout</button>`
                : "—"
            }
          </td>
        </tr>
      `).join("")
      : `
        <tr>
          <td colspan="7" class="empty">
            No bookings found.
          </td>
        </tr>
      `;
}

function renderRecent() {
  const rows = bookings.slice().reverse().slice(0, 5);

  document.getElementById("recent").innerHTML =
    rows.length
      ? rows.map(booking => `
        <tr>
          <td>#${booking.id}</td>
          <td>${esc(booking.name)}</td>

          <td>
            ${booking.room} · ${roomLabel(booking)}
          </td>

          <td>${booking.nights} night(s)</td>

          <td>
            <span class="badge ${booking.active ? "" : "closed"}">
              ${booking.active ? "Active" : "Closed"}
            </span>
          </td>
        </tr>
      `).join("")
      : `
        <tr>
          <td colspan="5" class="empty">
            No bookings yet.
          </td>
        </tr>
      `;
}

function renderFood() {
  document.getElementById("menu").innerHTML =
    FOODS.map(food => `
      <div class="food">
        <span>${food.emoji} ${food.name}</span>
        <strong>${money(food.price)}</strong>
      </div>
    `).join("");

  const bookingSelect = document.getElementById(
    "food-booking"
  );

  const oldBooking = bookingSelect.value;

  bookingSelect.innerHTML = active().map(booking => `
    <option value="${booking.id}">
      #${booking.id} — ${esc(booking.name)} — Room ${booking.room}
    </option>
  `).join("");

  if (!active().length) {
    bookingSelect.innerHTML =
      '<option value="">No active bookings</option>';
  }

  if (
    [...bookingSelect.options].some(
      option => option.value === oldBooking
    )
  ) {
    bookingSelect.value = oldBooking;
  }

  const foodSelect = document.getElementById("food-item");
  const oldFood = foodSelect.value;

  foodSelect.innerHTML = FOODS.map((food, index) => `
    <option value="${index}">
      ${food.name} — ${money(food.price)}
    </option>
  `).join("");

  if (
    [...foodSelect.options].some(
      option => option.value === oldFood
    )
  ) {
    foodSelect.value = oldFood;
  }

  updateFoodTotal();
}

function updateFoodTotal() {
  const index = Number(
    document.getElementById("food-item").value
  ) || 0;

  const quantity = Number(
    document.getElementById("food-qty").value
  ) || 1;

  document.getElementById("food-total").textContent =
    money(FOODS[index].price * quantity);
}

document.getElementById("food-item").addEventListener(
  "change",
  updateFoodTotal
);

document.getElementById("food-qty").addEventListener(
  "input",
  updateFoodTotal
);

document.getElementById("food-form").addEventListener(
  "submit",
  event => {
    event.preventDefault();

    const id = Number(
      document.getElementById("food-booking").value
    );

    const index = Number(
      document.getElementById("food-item").value
    );

    const quantity = Number(
      document.getElementById("food-qty").value
    );

    const booking = bookings.find(
      item => item.id === id && item.active
    );

    if (!booking) {
      notify("Create an active booking first.");
      return;
    }

    if (
      !Number.isInteger(quantity) ||
      quantity < 1 ||
      quantity > 50
    ) {
      notify("Quantity must be 1–50.");
      return;
    }

    const food = FOODS[index];

    booking.foodBill += food.price * quantity;

    booking.orders.push({
      name: food.name,
      quantity,
      total: food.price * quantity
    });

    save();

    document.getElementById("food-qty").value = 1;

    render();

    notify("Food order added to bill.");
  }
);

function renderBillOptions() {
  const select = document.getElementById("bill-booking");
  const oldValue = select.value;

  select.innerHTML = active().map(booking => `
    <option value="${booking.id}">
      #${booking.id} — ${esc(booking.name)} — Room ${booking.room}
    </option>
  `).join("");

  if (!active().length) {
    select.innerHTML =
      '<option value="">No active bookings</option>';
  }

  if (
    [...select.options].some(
      option => option.value === oldValue
    )
  ) {
    select.value = oldValue;
  }
}

document.getElementById("bill-form").addEventListener(
  "submit",
  event => {
    event.preventDefault();

    const id = Number(
      document.getElementById("bill-booking").value
    );

    const booking = bookings.find(
      item => item.id === id && item.active
    );

    if (!booking) {
      notify("Create an active booking first.");
      return;
    }

    showInvoice(booking);
  }
);

function showInvoice(booking) {
  const room = TYPES[booking.type].price * booking.nights;
  const subtotal = room + booking.foodBill;
  const tax = subtotal * 0.05;
  const total = subtotal + tax;

  const invoice = document.getElementById("invoice");

  invoice.className = "panel";

  invoice.innerHTML = `
    <h2>GRAND STAY</h2>
    <p>Invoice #${booking.id}</p>

    <p>
      Guest: ${esc(booking.name)} |
      Phone: ${esc(booking.phone)}
    </p>

    <p>
      Room ${booking.room} ·
      ${roomLabel(booking)} ·
      ${booking.nights} night(s)
    </p>

    <div class="total">
      <span>Room Charges</span>
      <span>${money(room)}</span>
    </div>

    <div class="total">
      <span>Food Charges</span>
      <span>${money(booking.foodBill)}</span>
    </div>

    <div class="total">
      <span>Subtotal</span>
      <span>${money(subtotal)}</span>
    </div>

    <div class="total">
      <span>Assumed Tax (5%)</span>
      <span>${money(tax)}</span>
    </div>

    <div class="total">
      <span>Total Due</span>
      <span>${money(total)}</span>
    </div>

    <button class="secondary" onclick="window.print()">
      Print Invoice
    </button>

    <button
      class="primary"
      onclick="checkout(${booking.id})"
    >
      Confirm Checkout
    </button>

    <p class="invoice-note">
      Educational demo; no payment is processed.
    </p>
  `;
}

function checkout(id) {
  const booking = bookings.find(
    item => item.id === id && item.active
  );

  if (!booking) {
    notify("Booking is not active.");
    return;
  }

  const confirmed = confirm(
    "Check out " +
    booking.name +
    " from room " +
    booking.room +
    "?"
  );

  if (!confirmed) {
    return;
  }

  booking.active = false;

  save();
  render();

  const invoice = document.getElementById("invoice");

  invoice.className = "empty";
  invoice.textContent = "Checkout completed. Room released.";

  notify("Checkout completed.");
}

render();