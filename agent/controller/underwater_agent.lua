function init()
    state = {
        est_x = 0,
        est_y = 0
    }
end

function step()

    local x, y = get_position()

    -- noisy observation (sensor model)
    state.est_x = x + math.random() * 0.1
    state.est_y = y + math.random() * 0.1

    local msg = {}

    msg.sender = robot.id
    msg.est_x = state.est_x
    msg.est_y = state.est_y

    -- OPTIONAL: confidence (utile per EKF / consensus / PF)
    msg.conf = 1.0

    -- routing decision (layer 3 separato)
    local dest = "fb_0"
    local next_hop = get_next_hop(dest)

    if next_hop ~= nil and next_hop ~= "" then
        msg.receiver = next_hop
        send_message(msg)
    end
end