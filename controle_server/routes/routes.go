package routes

import (
	"OpenFabControl/handler/resource_handler"
	"OpenFabControl/handler/role_handler"
	"OpenFabControl/handler/session_handler"
	"OpenFabControl/handler/user_handler"
	"fmt"
	"net/http"
)

// function to manage routes
func Setup_routes() {
	//////////////////////////////
	// machine controler routes //
	//////////////////////////////

	http.HandleFunc("/machine-api/register", resource_handler.Register)
	http.HandleFunc("/machine-api/check_approval_status", resource_handler.Check_approval_status)
	http.HandleFunc("/machine-api/create_session", session_handler.Create_session)
	http.HandleFunc("/machine-api/start_session", session_handler.Start_session)
	http.HandleFunc("/machine-api/stop_session", session_handler.Stop_session)
	http.HandleFunc("/machine-api/next_booking", session_handler.Next_booking)
	http.HandleFunc("/machine-api/add_time", session_handler.Add_time)
	http.HandleFunc("/machine-api/get_max_add_time", session_handler.Get_max_add_time)
	http.HandleFunc("/machine-api/create_user", user_handler.Create_user)

	///////////////////////
	// admin page routes //
	///////////////////////

	// admin auth
	http.HandleFunc("/web-admin-api/login", user_handler.AdminLogin)

	// machine controlers
	http.HandleFunc("/web-admin-api/get_resource_list_to_approve", admin_middleware(resource_handler.Get_resource_list_to_approve))
	http.HandleFunc("/web-admin-api/get_resource_list_approved", admin_middleware(resource_handler.Get_resource_list_approved))
	http.HandleFunc("/web-admin-api/approve_resource", admin_middleware(resource_handler.Approve_resource))
	http.HandleFunc("/web-admin-api/unapprove_resource", admin_middleware(resource_handler.Unapprove_resource))
	http.HandleFunc("/web-admin-api/delete_resource", admin_middleware(resource_handler.Delete_resource))
	http.HandleFunc("/web-admin-api/edit_resource", admin_middleware(resource_handler.Edit_resource))

	http.HandleFunc("/web-admin-api/create_session", admin_middleware(session_handler.Create_session))
	http.HandleFunc("/web-admin-api/fetch_booking", admin_middleware(session_handler.Fetch_booking))

	// users
	http.HandleFunc("/web-admin-api/create_user", admin_middleware(user_handler.Create_user))
	http.HandleFunc("/web-admin-api/get_user_list", admin_middleware(user_handler.Get_user_list))
	http.HandleFunc("/web-admin-api/delete_user", admin_middleware(user_handler.Delete_user))
	http.HandleFunc("/web-admin-api/update_user", admin_middleware(user_handler.Update_user))
	http.HandleFunc("/web-admin-api/desactivate_user", admin_middleware(user_handler.Desactivate_user))
	http.HandleFunc("/web-admin-api/reactivate_user", admin_middleware(user_handler.Reactivate_user))
	http.HandleFunc("/web-admin-api/get_user_roles", admin_middleware(user_handler.Get_user_roles))
	// roles
	http.HandleFunc("/web-admin-api/get_role_list", admin_middleware(role_handler.Get_role_list))
	http.HandleFunc("/web-admin-api/create_role", admin_middleware(role_handler.Create_role))
	http.HandleFunc("/web-admin-api/delete_role", admin_middleware(role_handler.Delete_role))
	http.HandleFunc("/web-admin-api/assign_role_to_user", admin_middleware(role_handler.Assign_role_to_user))
	http.HandleFunc("/web-admin-api/remove_role_from_user", admin_middleware(role_handler.Remove_role_from_user))

	///////////////////////
	// user pages routes //
	///////////////////////

	http.HandleFunc("/web-user-api/user_one_time_setup", user_handler.User_one_time_setup)
	http.HandleFunc("/web-user-api/create_session", auth_middleware(session_handler.Create_session))
	http.HandleFunc("/web-user-api/fetch_booking", auth_middleware(session_handler.Fetch_booking))
	http.HandleFunc("/web-user-api/fetch_my_booking", auth_middleware(session_handler.Fetch_my_booking))
	http.HandleFunc("/web-user-api/login", user_handler.Login)

	///////////
	// other //
	///////////

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintf(w, "controle server working")
	})
}
